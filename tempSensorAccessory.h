#pragma once
#ifndef __TEMPERATURESENSORACCESSORY_H__
#define __TEMPERATURESENSORACCESSORY_H__

#include "common.h"
#include <HomekitRemoteDeviceServerSide.h>
#include <HomeSpan.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

struct TempSensorAccessory : HomekitRemoteDeviceServerSide, Service::TemperatureSensor {
    SpanCharacteristic *curTemp;

    TempSensorAccessory(WebSocketsServer *inWS) : HomekitRemoteDeviceServerSide(inWS), Service::TemperatureSensor() {
        curTemp = new Characteristic::CurrentTemperature();
    }

    boolean update() {
        return true;
    }

    void loop() {
        listenForHKRResponse();
    }

    void handleHKRCommand(const JsonDocument &doc) {
        const char *command = doc["command"].as<const char *>();
        bool success = false;

        if (strcmp(command, TS_COMMAND_UPDATE_TEMP) == 0) {
            HK_LOG_LINE("Updating homekit from Inkbird temperature sensor.");
            curTemp->setVal(doc["payload"].as<float>());
            success = true;
        }

        StaticJsonDocument<92> resDoc;
        resDoc["device"] = TS_MODEL;
        resDoc["command"] = HKR_RESPONSE_COMMAND;
        resDoc["payload"] = success;
        sendHKRMessage(resDoc, false);

        if (!success) HK_ERROR_LINE("Error handling temerature sensor command %s.", command);
    }
};

#endif