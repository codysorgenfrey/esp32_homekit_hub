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

    const char* handleMessage(const JsonDocument &doc) {
        HK_LOG_LINE("Updating homekit from Inkbird temperature sensor.");
        // only accept one message, update temp.
        String command = doc["command"].as<String>();
        if (command == String("update_temp")) {
            curTemp->setVal(doc["payload"].as<float>());
            return "Success";
        }

        HK_ERROR_LINE("Error handling temerature sensor message.");
        return "Error";
    }
};

#endif