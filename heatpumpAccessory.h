#pragma once
#ifndef __HEATPUMPACCESSORY_H__
#define __HEATPUMPACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define ACTIVE_OFF 0
#define ACTIVE_ON 1
#define CURRENTFANSTATE_INACTIVE 0
#define CURRENTFANSTATE_IDLE 1
#define CURRENTFANSTATE_BLOWING 2
#define TARGETFANSTATE_MANUAL 0
#define TARGETFANSTATE_AUTO 1
#define SLATTYPE_HORIZONTAL 0
#define SLATTYPE_VERTICAL 1
#define CURRENTSLATSTATE_FIXED 0
#define CURRENTSLATSTATE_JAMMED 1
#define CURRENTSLATSTATE_SWINGING 2
#define SWINGMODE_DISABLED 0
#define SWINGMODE_SWINGING 1
#define CURRENTHEATERCOOLERSTATE_INACTIVE 0
#define CURRENTHEATERCOOLERSTATE_IDLE 1
#define CURRENTHEATERCOOLERSTATE_HEATING 2
#define CURRENTHEATERCOOLERSTATE_COOLING 3
#define TARGETHEATERCOOLERSTATE_AUTO 0
#define TARGETHEATERCOOLERSTATE_HEATING 1
#define TARGETHEATERCOOLERSTATE_COOLING 2
#define TARGETHEATERCOOLERSTATE_OFF 3
#define SLATANGLE_ALLUP -90
#define SLATANGLE_ALLDOWN 90

bool updateSettings = false;

struct FanAccessory : Service::Fan {
    SpanCharacteristic *active = new Characteristic::Active();
    SpanCharacteristic *curState = new Characteristic::CurrentFanState(); // 0 inactive, 1 idle, 2 blowing
    SpanCharacteristic *tarState = new Characteristic::TargetFanState(); // 0 manual, 1 auto
    SpanCharacteristic *speed = new Characteristic::RotationSpeed(); // 0-100 percentage

    boolean update() {
        updateSettings = true;
        return updateSettings;
    }
};

struct SlatsAccessory : Service::Slat {
    SpanCharacteristic *type = new Characteristic::SlatType(SLATTYPE_VERTICAL); // 0 horizontal, 1 vertical
    SpanCharacteristic *curState = new Characteristic::CurrentSlatState(); // 0 fixed, 1 jammed, 2 swinging
    SpanCharacteristic *curAngle = new Characteristic::CurrentTiltAngle(); // -90 to 90 where -90 is straight out and 90 is straight down
    SpanCharacteristic *tarAngle = new Characteristic::TargetTiltAngle(); // -90 to 90 where -90 is straight out and 90 is straight down
    SpanCharacteristic *swing = new Characteristic::SwingMode(); // 0 disabled, 1 enabled

    boolean update() {
        updateSettings = true;
        return updateSettings;
    }
};

struct HeatpumpAccessory : Service::HeaterCooler {
    SpanCharacteristic *active;
    SpanCharacteristic *curTemp;
    SpanCharacteristic *curState; // 0 inactive, 1 idle, 2 heating, 3 cooling
    SpanCharacteristic *tarState; // 0 auto, 1 heating, 2 cooling, 3 off
    SpanCharacteristic *coolingThreshold; // 10-35
    SpanCharacteristic *heatingThreshold; // 0-25
    FanAccessory *fan;
    SlatsAccessory *slats;
    WebSocketsServer *webSocket;
    const char *serial;
    bool initialized = false;

    HeatpumpAccessory(WebSocketsServer *inWebSocket, const char *inSerial) : Service::HeaterCooler() {
        active = new Characteristic::Active();
        curTemp = new Characteristic::CurrentTemperature();
        curState = new Characteristic::CurrentHeaterCoolerState();
        tarState = new Characteristic::TargetHeaterCoolerState();
        coolingThreshold = new Characteristic::CoolingThresholdTemperature();
        heatingThreshold = new Characteristic::HeatingThresholdTemperature();
        fan = new FanAccessory();
        slats = new SlatsAccessory();
        webSocket = inWebSocket;
        serial = inSerial;
    }

    float vaneAngleFromSetting(const char *setting) {
        float vanePos = atof(setting);
        return -90 + (((vanePos - 1) / 4) * 180);
    }

    String settingFromVaneAngle(float angle) {
        int setting = static_cast<int>(round((((angle + 90.0f) / 180.0f) * 4.0f) + 1.0f));
        return String(setting);
    }

    float fanSpeedFromSetting(const char *setting) {
        return atof(setting) * 25.0f;
    }

    String settingFromFanSpeed(float speed) {
        int setting = static_cast<int>(round(speed / 25.0f));
        if (setting == 0) return String("QUIET");
        return String(setting);
    }

    boolean update() {
        HK_LOG_LINE("Updating %s from Homekit.", serial);
        StaticJsonDocument<192> doc;
        doc["device"] = serial;
        doc["command"] = "update_settings";
        
        if (active->getNewVal() == 1) doc["payload"]["power"] = "ON";
        else doc["payload"]["power"] = "OFF";

        switch (tarState->getNewVal()) {
            case TARGETHEATERCOOLERSTATE_HEATING:
                doc["payload"]["mode"] = "HEAT";
                break;

            case TARGETHEATERCOOLERSTATE_COOLING:
                doc["payload"]["mode"] = "COOL";
                break;
            
            case TARGETHEATERCOOLERSTATE_AUTO:
                doc["payload"]["mode"] = "AUTO";
                break;
            
            default:
                HK_ERROR_LINE("Heatpump %s unknown target state: %d", serial, tarState->getNewVal());
                break;
        }

        if (tarState->getNewVal() == TARGETHEATERCOOLERSTATE_COOLING) {
            doc["payload"]["temperature"] = coolingThreshold->getNewVal();
        } else if (tarState->getNewVal() == TARGETHEATERCOOLERSTATE_HEATING) {
            doc["payload"]["temperature"] = heatingThreshold->getNewVal();
        } else if (tarState->getNewVal() == TARGETHEATERCOOLERSTATE_AUTO) {
            float cool = coolingThreshold->getNewVal();
            float heat = heatingThreshold->getNewVal();
            float halfRange = (max(cool, heat) - min(cool, heat)) / 2;
            doc["payload"]["temperature"] = min(cool, heat) + halfRange;
        } else {
            HK_ERROR_LINE("Heatpump %s unknown target state: %d", serial, tarState->getNewVal());
        }

        if (fan->active->getNewVal() == ACTIVE_OFF) doc["payload"]["fan"] = "QUIET";
        if (fan->tarState->getNewVal() == TARGETFANSTATE_AUTO) doc["payload"]["fan"] = "AUTO";
        else doc["payload"]["fan"] = settingFromFanSpeed(fan->speed->getNewVal());

        if (slats->swing->getNewVal() == SWINGMODE_SWINGING) doc["payload"]["vane"] = "SWING";
        else doc["payload"]["vane"] = settingFromVaneAngle(slats->tarAngle->getNewVal());

        doc["payload"]["wideVane"] = "|";

        String message;
        serializeJson(doc, message);
        HK_VERB_LINE("Sending %s: %s", serial, message.c_str());

        return webSocket->broadcastTXT(message);
    }

    void loop() {
        if (updateSettings) {
            updateSettings = false; // clear state
            update();
        }
        if (millis() % 15000 == 0 && !initialized) { // once every 15 seconds
            HK_LOG_LINE("Looking for heatpump %s", serial);
            String message("{'device':'");
            message += serial;
            message += "', 'command':'get_settings'}";
            webSocket->broadcastTXT(message);
        }
    }

    bool updateTargetState(const JsonDocument &doc) {
        String power = doc["payload"]["power"].as<String>();
        if (power == String("ON")) {
            active->setVal(ACTIVE_ON);
            fan->active->setVal(ACTIVE_ON);
        } else {
            active->setVal(ACTIVE_OFF);
            fan->active->setVal(ACTIVE_OFF);
        }

        String mode = doc["payload"]["mode"].as<String>();
        if (mode == String("HEAT")) tarState->setVal(TARGETHEATERCOOLERSTATE_HEATING);
        else if (mode == String("COOL")) tarState->setVal(TARGETHEATERCOOLERSTATE_COOLING);
        else tarState->setVal(TARGETHEATERCOOLERSTATE_AUTO); // default to auto

        float temperature = doc["payload"]["temperature"].as<float>();
        if (tarState->getVal() == TARGETHEATERCOOLERSTATE_COOLING) coolingThreshold->setVal(temperature);
        else if (tarState->getVal() == TARGETHEATERCOOLERSTATE_HEATING) heatingThreshold->setVal(temperature);
        else { 
            // it seems the HP tries to keep within half a degree C
            coolingThreshold->setVal(temperature + 0.5);
            heatingThreshold->setVal(temperature - 0.5);
        }

        String fanSetting = doc["payload"]["fan"].as<String>();
        if (fanSetting == String("AUTO")) fan->tarState->setVal(TARGETFANSTATE_AUTO);
        else if (fanSetting == String("QUIET")) {
            fan->tarState->setVal(TARGETFANSTATE_MANUAL);
            fan->speed->setVal(fanSpeedFromSetting("1"));
        } else {
            fan->tarState->setVal(TARGETFANSTATE_MANUAL);
            fan->speed->setVal(fanSpeedFromSetting(fanSetting.c_str()));
        }

        String vane = doc["payload"]["vane"].as<String>();
        if (vane == String("AUTO")) {
            slats->swing->setVal(SWINGMODE_DISABLED);
            if (curTemp->getVal() < temperature) { // heating
                slats->tarAngle->setVal(SLATANGLE_ALLDOWN);
            } else {
                slats->tarAngle->setVal(SLATANGLE_ALLUP);
            }
        } else if (vane == String("SWING")) {
            slats->swing->setVal(SWINGMODE_SWINGING);
        } else {
            slats->swing->setVal(SWINGMODE_DISABLED);
            slats->tarAngle->setVal(vaneAngleFromSetting(vane.c_str()));
        }

        HK_VERB_LINE("Success replacing target settings for %s", serial);
        return true;
    }

    bool updateCurrentState(const JsonDocument &doc) {
        String power = doc["payload"]["power"].as<String>();
        if (power == String("OFF")) {
            curState->setVal(CURRENTHEATERCOOLERSTATE_INACTIVE);
            fan->curState->setVal(CURRENTFANSTATE_INACTIVE);
        }

        float temperature = doc["payload"]["temperature"].as<float>();
        String vane = doc["payload"]["vane"].as<String>();
        if (vane == String("AUTO")) {
            slats->curState->setVal(CURRENTSLATSTATE_FIXED);
            if (curTemp->getVal() < temperature) { // heating
                slats->curAngle->setVal(SLATANGLE_ALLDOWN);
            } else {
                slats->curAngle->setVal(SLATANGLE_ALLUP);
            }
        } else if (vane == String("SWING")) {
            slats->curState->setVal(CURRENTSLATSTATE_SWINGING);
        } else {
            slats->curState->setVal(CURRENTSLATSTATE_FIXED);
            slats->curAngle->setVal(vaneAngleFromSetting(vane.c_str()));
        }

        HK_VERB_LINE("Success updating settings for %s", serial);
        return true;
    }

    bool updateStatus(const JsonDocument &doc) {
        curTemp->setVal(doc["payload"]["roomTemperature"].as<float>());

        bool operating = doc["payload"]["operating"].as<bool>();
        if (operating) { 
            if (tarState->getVal() == TARGETHEATERCOOLERSTATE_HEATING) curState->setVal(CURRENTHEATERCOOLERSTATE_HEATING);
            else if (tarState->getVal() == TARGETHEATERCOOLERSTATE_COOLING) curState->setVal(CURRENTHEATERCOOLERSTATE_COOLING);
            else { // for auto mode
                if (curTemp->getVal() >= coolingThreshold->getVal()) curState->setVal(CURRENTHEATERCOOLERSTATE_COOLING);
                else if (curTemp->getVal() <= heatingThreshold->getVal()) curState->setVal(CURRENTHEATERCOOLERSTATE_HEATING);
            }
            fan->curState->setVal(CURRENTFANSTATE_BLOWING);
        } else {
            curState->setVal(CURRENTHEATERCOOLERSTATE_IDLE); 
            fan->curState->setVal(CURRENTFANSTATE_IDLE);
        }

        HK_VERB_LINE("Success updating status for %s", serial);
        return true;
    }

    const char* handleMessage(const JsonDocument &doc) {
        HK_LOG_LINE("Updating Homekit from %s", serial);
        HK_VERB_LINE("%s", doc.as<String>().c_str());   

        String command = doc["command"].as<String>();
        if (command == String("update_settings")) {
            if (updateCurrentState(doc)) return "Success";
        } else if (command == String("replace_settings")) {
            if (updateTargetState(doc) && updateCurrentState(doc)) {
                if (!initialized) initialized = true;
                return "Success";
            }
        } else if (command == String("update_status")) {
            if (updateStatus(doc)) return "Success";
        }

        HK_ERROR_LINE("Error handling heatpump message.");
        return "Error";
    }
};

#endif