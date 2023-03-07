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

    TempSensorAccessory(WebSocketsServer *inWS) : HomekitRemoteDeviceServerSide(inWS, TS_MODEL), Service::TemperatureSensor() {
        curTemp = new Characteristic::CurrentTemperature();
    }

    boolean update() {
        return true;
    }

    void loop() {
        listenForHKRResponse();
    }

    void handleHKRCommand(const char *command, const JsonVariant &payload) {
        bool success = false;

        if (strcmp(command, TS_COMMAND_UPDATE_TEMP) == 0) {
            HK_LOG_LINE("Updating homekit from Inkbird temperature sensor.");
            curTemp->setVal(payload.as<float>());
            success = true;
        }

        sendHKRResponse(success);
        if (!success) HK_ERROR_LINE("Error handling temerature sensor command %s.", command);
    }

    void handleHKRError(HKR_ERROR err) {
        switch (err) {
        case HKR_ERROR_TIMEOUT:
            HK_ERROR_LINE("%s: HKR timeout waiting for response.", TS_MODEL);
            break;
        case HKR_ERROR_UNEXPECTED_RESPONSE:
            HK_ERROR_LINE("%s: HKR response recieved to no command.", TS_MODEL);
            break;
        case HKR_ERROR_WEBSOCKET_ERROR:
            HK_ERROR_LINE("%s: HKR websocket error.", TS_MODEL);
            break;
        case HKR_ERROR_CONNECTION_REFUSED:
            HK_ERROR_LINE("%s: HKR refused connection.", TS_MODEL);
            break;
        case HKR_ERROR_DEVICE_NOT_REGISTERED:
            HK_ERROR_LINE("%s: HKR device not registered.", TS_MODEL);
            break;
        case HKR_ERROR_JSON_DESERIALIZE:
            HK_ERROR_LINE("%s: HKR json deserialize error.", TS_MODEL);
            break;
        default:
            HK_ERROR_LINE("%s: HKR unknown error: %i.", TS_MODEL, err);
            break;
        }
    }
};

#endif