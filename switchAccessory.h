#pragma once
#ifndef __SWITCHACCESSORY_H__
#define __SWITCHACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>

#define WM_COMMAND_UPDATE_ON "update_on"

struct WeMoSwitchAccessory : Service::Switch {
  SpanCharacteristic *on;
  WebSocketsServer *webSocket;
  
  WeMoSwitchAccessory(WebSocketsServer *ws) : Service::Switch() {
    on = new Characteristic::On();
    webSocket = ws;
  }

  boolean update() {
    boolean success = true;
    if (on->updated()) success &= setOnState();
    return success;
  }

  bool setOnState() {
    StaticJsonDocument<92> doc;
    doc["device"] = WM_DEVICE_ID;
    doc["command"] = WM_COMMAND_UPDATE_ON;
    doc["payload"] = on->getNewVal();

    String message;
    serializeJson(doc, message);

    HK_VERB_LINE("Sending message to WeMo: %s", message);
    return webSocket->broadcastTXT(message);
  }

  void respondToMessage(bool success) {
    StaticJsonDocument<92> doc;
    doc["device"] = WM_DEVICE_ID;
    doc["command"] = HUB_COMMAND_RESPONSE;
    doc["payload"] = success ? HUB_RESPONSE_OK : HUB_RESPONSE_ERROR;

    String message;
    serializeJson(doc, message);

    HK_VERB_LINE("Sending message to WeMo: %s", message);
    webSocket->broadcastTXT(message);
  }

  void handleMessage(u_int8_t *message) {
    StaticJsonDocument<92> doc;
    DeserializationError err = deserializeJson(doc, message);

    if (err) {
      HK_ERROR_LINE("Error deserializing message: %s", err.c_str());
      respondToMessage(false);
    }

    if (doc['device'].as<const char *>() == WM_DEVICE_ID) {
      if (doc['command'].as<const char *>() == WM_COMMAND_UPDATE_ON) {
        HK_LOG_LINE("Updating homekit from WeMo switch.");
        on->setVal(doc['payload'].as<bool>());
        respondToMessage(true);
      }
    }
  }
};

#endif