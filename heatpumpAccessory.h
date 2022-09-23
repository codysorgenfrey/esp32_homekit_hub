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

struct FanAccessory : Service::Fan {
    SpanCharacteristic *active = new Characteristic::Active();
    SpanCharacteristic *curState = new Characteristic::CurrentFanState(); // 0 inactive, 1 idle, 2 blowing
    SpanCharacteristic *tarState = new Characteristic::TargetFanState(); // 0 manual, 1 auto
    SpanCharacteristic *speed = new Characteristic::RotationSpeed(); // 0-100 percentage
};

struct SlatsAccessory : Service::Slat {
    SpanCharacteristic *type = new Characteristic::SlatType(SLATTYPE_VERTICAL); // 0 horizontal, 1 vertical
    SpanCharacteristic *curState = new Characteristic::CurrentSlatState(); // 0 fixed, 1 jammed, 2 swinging
    SpanCharacteristic *curAngle = new Characteristic::CurrentTiltAngle(); // -90 to 90 where -90 is straight out and 90 is straight down
    SpanCharacteristic *tarAngle = new Characteristic::TargetTiltAngle(); // -90 to 90 where -90 is straight out and 90 is straight down
    SpanCharacteristic *swing = new Characteristic::SwingMode(); // 0 disabled, 1 enabled
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

    boolean update() {
        HK_LOG_LINE("Update %s from Homekit.", serial);
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
            
            default:
                doc["payload"]["mode"] = "AUTO";
                break;
        }

        float temp = heatingThreshold->getNewVal();
        if (tarState->getNewVal() == TARGETHEATERCOOLERSTATE_COOLING) temp = coolingThreshold->getNewVal();
        else {
            float halfRange = (coolingThreshold->getNewVal() - heatingThreshold->getNewVal()) / 2;
            temp = heatingThreshold->getNewVal() + halfRange;
        }
        doc["payload"]["temperature"] = temp;

        if (fan->active->getNewVal() == ACTIVE_OFF) doc["payload"]["fan"] = "QUIET";
        if (fan->tarState->getNewVal() == TARGETFANSTATE_AUTO) doc["payload"]["fan"] = "AUTO";
        else doc["payload"]["fan"] = String(static_cast<int>(round(fan->speed->getNewVal() / 25.0)));

        if (slats->swing->getNewVal() == SWINGMODE_SWINGING) doc["payload"]["vane"] = "SWING";
        else doc["payload"]["vane"] = String(static_cast<int>(round((((slats->tarAngle->getNewVal() + 90) / 180) * 4) + 1)));

        doc["payload"]["wideVane"] = "|";

        String message;
        serializeJson(doc, message);
        HK_LOG_LINE("Sending %s: %s", serial, message.c_str());

        return webSocket->broadcastTXT(message);
    }

    const char* handleMessage(const JsonDocument &doc) {
        HK_LOG_LINE("Updating homekit from %s", serial);
        String command = doc["command"].as<String>();
        if (command == String("update_settings")) {
            String power = doc["payload"]["power"].as<String>();
            if (power == String("ON")) {
                active->setVal(ACTIVE_ON);
                fan->active->setVal(ACTIVE_ON);
            } else {
                active->setVal(ACTIVE_OFF);
                curState->setVal(CURRENTHEATERCOOLERSTATE_INACTIVE);
                fan->active->setVal(ACTIVE_OFF);
                fan->curState->setVal(CURRENTFANSTATE_INACTIVE);
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
                fan->speed->setVal(25.0);
            } else {
                fan->tarState->setVal(TARGETFANSTATE_MANUAL);
                fan->speed->setVal(atof(fanSetting.c_str()) * 25.0);
            }

            String vane = doc["payload"]["vane"].as<String>();
            if (vane == String("AUTO")) {
                slats->swing->setVal(SWINGMODE_DISABLED);
                slats->curState->setVal(CURRENTSLATSTATE_FIXED);
                if (curTemp->getVal() < temperature) { // heating
                    slats->tarAngle->setVal(90);
                    slats->curAngle->setVal(90);
                } else {
                    slats->tarAngle->setVal(-90);
                    slats->curAngle->setVal(-90);
                }
            } else if (vane == String("SWING")) {
                slats->swing->setVal(SWINGMODE_SWINGING);
                slats->curState->setVal(CURRENTSLATSTATE_SWINGING);
            } else {
                slats->swing->setVal(SWINGMODE_DISABLED);
                slats->curState->setVal(CURRENTSLATSTATE_FIXED);
                float vanePos = atof(vane.c_str());
                float angle = -90 + (((vanePos - 1) / 4) * 180);
                slats->tarAngle->setVal(angle);
                slats->curAngle->setVal(angle);
            }

            HK_LOG_LINE("Success updating settings for %s", serial);
            return "Success";
        } else if (command == String("update_status")) {
            curTemp->setVal(doc["payload"]["roomTemperature"].as<float>());

            bool operating = doc["payload"]["operating"].as<bool>();
            if (operating) { 
                if (curTemp->getVal() >= coolingThreshold->getVal()) curState->setVal(CURRENTHEATERCOOLERSTATE_COOLING);
                else if (curTemp->getVal() <= heatingThreshold->getVal()) curState->setVal(CURRENTHEATERCOOLERSTATE_HEATING);
                fan->curState->setVal(CURRENTFANSTATE_BLOWING);
            } else {
                curState->setVal(CURRENTHEATERCOOLERSTATE_IDLE); 
                fan->curState->setVal(CURRENTFANSTATE_IDLE);
            }
            HK_LOG_LINE("Success updating status for %s", serial);
            return "Success";
        }

        HK_ERROR_LINE("Error handling heatpump message.");
        return "Error";
    }
};

#endif