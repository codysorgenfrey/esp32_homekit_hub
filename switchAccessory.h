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

    HK_VERB_LINE("Sending message to WeMo: %s", message.c_str());
    return webSocket->broadcastTXT(message);
  }

  void respondToMessage(bool success) {
    StaticJsonDocument<92> doc;
    doc["device"] = WM_DEVICE_ID;
    doc["command"] = HUB_COMMAND_RESPONSE;
    doc["payload"] = success ? HUB_RESPONSE_OK : HUB_RESPONSE_ERROR;

    String message;
    serializeJson(doc, message);

    HK_VERB_LINE("Sending message to WeMo: %s", message.c_str());
    webSocket->broadcastTXT(message);
  }

  void handleMessage(const JsonDocument &doc) {
    if (strcmp(doc['device'].as<const char *>(), WM_DEVICE_ID) == 0) {
      if (strcmp(doc['command'].as<const char *>(), WM_COMMAND_UPDATE_ON) == 0) {
        HK_LOG_LINE("Updating homekit from WeMo switch.");
        on->setVal(doc['payload'].as<bool>());
        // respondToMessage(true);
      }
    }
  }
};

#endif