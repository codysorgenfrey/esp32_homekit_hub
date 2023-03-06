#pragma once
#ifndef REMOTEHOMEKITDEVICE_H
#define REMOTEHOMEKITDEVICE_H

#include "common.h"
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

class HomekitRemoteDevice {
  WebSocketsServer *webSocket;
  unsigned long lastMessage = 0;
  bool needResponse = false;
  int clientID = -1;

public:
  HomekitRemoteDevice(WebSocketsServer *ws) {
    webSocket = ws;
  }

  virtual void handleHKRCommand(const JsonDocument &doc) = 0;

  void sendHKRMessage(const JsonDocument &doc, bool checkResponse = true) {
    String message;
    serializeJson(doc, message);
    if (!webSocket->sendTXT(clientID, message)) HK_ERROR_LINE("Error sending message: %s to: %i", message, clientID);
    lastMessage = millis();
    needResponse = checkResponse;
  }

  void HKRMessageRecieved(int id, const JsonDocument &doc) {
    clientID = id;

    const char *command = doc["command"].as<const char *>();
    if (strcmp(command, HKR_RESPONSE_COMMAND) == 0) {
      if (needResponse) {
        needResponse = false;
        lastMessage = 0;
      } else {
        HK_ERROR_LINE("Unexpected HKR response.");
      }
    } else {
      handleHKRCommand(doc);
    }
  }

  void listenForHKRResponse() {
    if (!needResponse) return;

    const unsigned long now = millis();
    const unsigned long diff = max(now, lastMessage) - min(now, lastMessage);
    if (diff >= HKR_RESPONSE_TIMEOUT) {
      HK_ERROR_LINE("HKR device not responding.");
      needResponse = false;
    }
  }
};

#endif