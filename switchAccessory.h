#ifndef __SWITCHACCESSORY_H__
#define __SWITCHACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#define WEMO_API "http://" WM_IP ":49153/upnp/control/basicevent1"

struct WeMoSwitchAccessory : Service::Switch {
    SpanCharacteristic *on;
    SpanCharacteristic *name;

    WeMoSwitchAccessory() : Service::Switch() {
        on = new Characteristic::On();
        name = new Characteristic::Name("WeMo Switch");
    }

    boolean update() {
        printDiagnostic();
        return updateWeMoState();
    }

    void loop() {
        // poll switch here for manual updates
        if (on->timeVal() >= WM_UPDATE_INTERVAL) updateHomekitState();
    }

    boolean updateWeMoState() {
        // Start First HTTP POST to retrieve switch status (0 for off, 1 for on)
        String stringState = on->getNewVal() ? "1" : "0";
        String postData = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>" + stringState + "</BinaryState></u:SetBinaryState></s:Body></s:Envelope>";
        
        WiFiClient client;
        HTTPClient http;
        http.begin(client, WEMO_API);
        http.addHeader("Content-Length", String(postData.length()));
        http.addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
        http.addHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#SetBinaryState\"");
        
        HK_LOG_LINE("Sending POST request to Wemo.");
        int httpCode = http.POST(postData);
        bool success = false;

        // If HTTP code is not negative, the POST succeeded
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                HK_LOG_LINE("Wemo switch state: %s.", stringState);
                success = true;
            } else {
                HK_LOG_LINE("Switch responded not ok.");
            }
        } else {
            HK_LOG_LINE("POST request error.");
        }

        http.end();
        client.stop();
        return success;
    }

    void updateHomekitState() {
        // tell homekit what switch says
        
        String postData = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"></u:GetBinaryState></s:Body></s:Envelope>";
        
        WiFiClient client;
        HTTPClient http;
        http.begin(client, WEMO_API);
        http.addHeader("Content-Length", String(postData.length()));
        http.addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
        http.addHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#GetBinaryState\"");
        
        HK_LOG_LINE("Sending POST request to Wemo.");
        int httpCode = http.POST(postData);

        // If HTTP code is not negative, the POST succeeded
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                const String &payload = http.getString();
                if (payload.indexOf("<BinaryState>1</BinaryState>") > 0) {
                    HK_LOG_LINE("Wemo switch on.");
                    if (!on->getVal()) on->setVal(true);
                } else {
                    HK_LOG_LINE("Wemo switch off.");
                    if (on->getVal()) on->setVal(false);
                }
            } else {
                HK_LOG_LINE("Switch responded not ok.");
            }
        } else {
            HK_LOG_LINE("POST request error.");
        }

        http.end();
        client.stop();
    }

    void printDiagnostic() {
        HK_LOG_LINE("WeMo on: %s", on->getNewVal() ? "on" : "off");
    }
};

#endif