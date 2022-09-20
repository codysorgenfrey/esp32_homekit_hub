#pragma once
#ifndef __TEMPERATURESENSORACCESSORY_H__
#define __TEMPERATURESENSORACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <ArduinoJson.h>

struct TempSensorAccessory : Service::TemperatureSensor {
    SpanCharacteristic *curTemp;

    TempSensorAccessory() : Service::TemperatureSensor() {
        curTemp = new Characteristic::CurrentTemperature();
    }

    boolean update() {
        return true;
    }

    bool handleMessage(const JsonDocument &doc) {
        // only accept one message, update temp.
        String command = doc["command"].as<String>();
        if (command == String("update_temp")) {
            curTemp->setVal(doc["payload"].as<float>());
            return true;
        }

        return false;
    }
};

#endif