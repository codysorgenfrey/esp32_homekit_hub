#ifndef __SWITCHACCESSORY_H__
#define __SWITCHACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#define WEMO_API "http://" WM_IP ":49153/upnp/control/basicevent1"

struct WeMoSwitchAccessory : Service::Switch {
    SpanCharacteristic *on;
    unsigned long lastPoll; // we have to track polling ourselves since we might not update each poll

    WeMoSwitchAccessory() : Service::Switch() {
        on = new Characteristic::On();
    }

    boolean update() {
        boolean success = true;
        if (on->updated()) success &= setOnState();
        return success;
    }

    void loop() {
        unsigned long now = millis();
        unsigned long timeDiff = max(now, lastPoll) - min(now, lastPoll);
        if (timeDiff >= WM_UPDATE_INTERVAL) pollOnState();
    }

    String request(String payload, String action) {
        WiFiClient *client = new WiFiClient();
        HTTPClient *http = new HTTPClient();
        http->begin(*client, WEMO_API);
        http->addHeader("Content-Length", String(payload.length()));
        http->addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
        http->addHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#" + action + "\"");
        
        int httpCode = http->POST(payload);

        String response;
        if (httpCode > 0) {
            if (httpCode >= 200 && httpCode <= 299) {
                response = http->getString();
            } else {
                HK_ERROR_LINE("Wemo sitch responded not ok.");
            }
        } else {
            HK_ERROR_LINE("Wemo POST request error.");
        }

        http->end();
        client->stop();
        delete http;
        delete client;
        return response;
    }

    void pollOnState() {        
        String postData = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"></u:GetBinaryState></s:Body></s:Envelope>";
        const String res = request(postData, "GetBinaryState");
        
        if (res.length() != 0) {
            lastPoll = millis();
            bool switchOn = res.indexOf("<BinaryState>1</BinaryState>") > 0;
            if (switchOn != on->getVal()) {
                HK_LOG_LINE("Found updated WeMo switch state.");
                on->setVal(switchOn);
            }
            return;
        }

        HK_ERROR_LINE("Could not poll Wemo state.");
    }

    boolean setOnState() {
        HK_LOG_LINE("WeMo set: %s", on->getNewVal() ? "on" : "off");
        
        String stringState = on->getNewVal() ? "1" : "0";
        String postData = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>" + stringState + "</BinaryState></u:SetBinaryState></s:Body></s:Envelope>";
        
        const String res = request(postData, "SetBinaryState");

        if (res.length() != 0) {
            return true;
        }

        HK_ERROR_LINE("Could not update Wemo state.");
        return false;
    }
};

#endif