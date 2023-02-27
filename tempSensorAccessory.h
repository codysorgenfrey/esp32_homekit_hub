#pragma once
#ifndef __TEMPERATURESENSORACCESSORY_H__
#define __TEMPERATURESENSORACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>

#define IB_COMMAND_UPDATE_TEMP "update_temp"

struct TempSensorAccessory : Service::TemperatureSensor {
    SpanCharacteristic *curTemp;
    WebSocketsServer *webSocket;

    TempSensorAccessory(WebSocketsServer *ws) : Service::TemperatureSensor() {
        curTemp = new Characteristic::CurrentTemperature();
        webSocket = ws;
    }

    boolean update() {
        return true;
    }

    void respondToMessage(bool success) {
      StaticJsonDocument<92> doc;
      doc["device"] = IB_DEVICE_ID;
      doc["command"] = HUB_COMMAND_RESPONSE;
      doc["payload"] = success ? HUB_RESPONSE_OK : HUB_RESPONSE_ERROR;

      String message;
      serializeJson(doc, message);

      HK_VERB_LINE("Sending message to Inkbird: %s", message);
      webSocket->broadcastTXT(message);
    }

    void handleMessage(u_int8_t *message) {
      StaticJsonDocument<92> doc;
      DeserializationError err = deserializeJson(doc, message);

      if (err) {
        HK_ERROR_LINE("Error deserializing message: %s", err.c_str());
        respondToMessage(false);
      }

      if (doc['device'].as<const char *>() == IB_DEVICE_ID) {
        if (doc['command'].as<const char *>() == IB_COMMAND_UPDATE_TEMP) {
          HK_LOG_LINE("Updating homekit from Inkbird temperature sensor.");
          curTemp->setVal(doc['payload'].as<float>());
          respondToMessage(true);
        }
      }
    }
};

#endif