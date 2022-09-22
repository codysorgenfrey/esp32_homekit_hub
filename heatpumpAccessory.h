#pragma once
#ifndef __HEATPUMPACCESSORY_H__
#define __HEATPUMPACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <ArduinoJson.h>

struct FanAccessory : Service::Fan {
    SpanCharacteristic *active = new Characteristic::Active();
    SpanCharacteristic *curState = new Characteristic::CurrentFanState(); // 0 inactive, 1 idle, 2 blowing
    SpanCharacteristic *tarState = new Characteristic::TargetFanState(); // 0 manual, 1 auto
};

struct SlatsAccessory : Service::Slat {
    SpanCharacteristic *type = new Characteristic::SlatType(); // 0 horizontal, 1 vertical
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
    FanAccessory *fan = new FanAccessory();
    SlatsAccessory *slats = new SlatsAccessory();

    HeatpumpAccessory() : Service::HeaterCooler() {
        active = new Characteristic::Active();
        curTemp = new Characteristic::CurrentTemperature();
        curState = new Characteristic::CurrentHeatingCoolingState();
        tarState = new Characteristic::TargetHeaterCoolerState();
        coolingThreshold = new Characteristic::CoolingThresholdTemperature();
        heatingThreshold = new Characteristic::HeatingThresholdTemperature();
    }

    boolean update() {
        return true;
    }

    const char* handleMessage(const JsonDocument &doc) {
        // only accept one message, update temp.
        String command = doc["command"].as<String>();
        if (command == String("update_temp")) {
            curTemp->setVal(doc["payload"].as<float>());
            return "Success";
        }

        HK_ERROR_LINE("Error handling heatpump message.");
        return "Error";
    }
};

#endif