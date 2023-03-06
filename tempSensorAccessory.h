#pragma once
#ifndef __TEMPERATURESENSORACCESSORY_H__
#define __TEMPERATURESENSORACCESSORY_H__

#include "common.h"
#include "remoteHomekitDevice.h"
#include <HomeSpan.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

struct RemoteTempSensor : Service::TemperatureSensor {};

struct TempSensorAccessory : HomekitRemoteDevice, Service::TemperatureSensor {
    SpanCharacteristic *curTemp;

    TempSensorAccessory(WebSocketsServer *inWS) : HomekitRemoteDevice(inWS), Service::TemperatureSensor() {
        curTemp = new Characteristic::CurrentTemperature();
    }

    boolean update() {
        return true;
    }

    void handleHKRCommand(const JsonDocument &doc) {
        const char *command = doc["command"].as<const char *>();

        if (strcmp(command, TS_COMMAND_UPDATE_TEMP) == 0) {
            HK_LOG_LINE("Updating homekit from Inkbird temperature sensor.");
            curTemp->setVal(doc["payload"].as<float>());
            return;
        }

        HK_ERROR_LINE("Error handling temerature sensor command %s.", command);
    }
};

#endif