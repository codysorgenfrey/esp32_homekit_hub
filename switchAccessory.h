#ifndef __SWITCHACCESSORY_H__
#define __SWITCHACCESSORY_H__

#include "common.h"
#include <homekit/characteristics.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

extern "C" homekit_characteristic_t switchOn;

HTTPClient *switchHttps;
WiFiClientSecure *switchClient;

void setSwitch(const homekit_value_t value) {
    // tell switch what homekit says

    // Start First HTTP POST to retrieve switch status (0 for off, 1 for on)
    String stringState = value.bool_value ? "1" : "0";
    String postData = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>" + stringState + "</BinaryState></u:SetBinaryState></s:Body></s:Envelope>";
    
    switchHttps->begin(*switchClient, (String) "http://" + WEMO_IP + ":49153/upnp/control/basicevent1");
    switchHttps->addHeader("Content-Length", (String)postData.length());
    switchHttps->addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
    switchHttps->addHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#SetBinaryState\"");
    
    HK_LOG_LINE("Sending POST request to Wemo.");
    int httpCode = switchHttps->POST(postData);

    // If HTTP code is not negative, the POST succeeded
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            HK_LOG_LINE("Wemo switch on.");
            switchOn.value = value;
        } else {
            HK_LOG_LINE("Switch responded not ok.");
        }
    } else {
        HK_LOG_LINE("POST request error.");
    }

    switchHttps->end(); // these might kill the WiFi... we'll see
    switchClient->stop();
}

homekit_value_t getSwitch() {
    // tell homekit what switch says
    
    // Start First HTTP POST to retrieve switch status (0 for off, 1 for on)
    String postData = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"></u:GetBinaryState></s:Body></s:Envelope>";
    
    WiFiClient client;
    HTTPClient http;
    switchHttps->begin(*switchClient, (String) "http://" + WEMO_IP + ":49153/upnp/control/basicevent1");
    switchHttps->addHeader("Content-Length", (String)postData.length());
    switchHttps->addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
    switchHttps->addHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#GetBinaryState\"");
    
    HK_LOG_LINE("Sending POST request to Wemo.");
    int httpCode = switchHttps->POST(postData);

    // If HTTP code is not negative, the POST succeeded
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            const String &payload = switchHttps->getString();
            if (payload.indexOf("<BinaryState>1</BinaryState>") > 0) {
                HK_LOG_LINE("Wemo switch on.");
                switchOn.value.bool_value = true;
            } else {
                HK_LOG_LINE("Wemo switch off.");
                switchOn.value.bool_value = false;
            }
        } else {
            HK_LOG_LINE("Switch responded not ok.");
        }
    } else {
        HK_LOG_LINE("POST request error.");
    }

    switchHttps->end();
    switchClient->stop();
    return switchOn.value;
}

bool initSwitchAccessory(HTTPClient *https, WiFiClientSecure *client) {
    switchHttps = https;
    switchClient = client;

    switchOn.setter = setSwitch;
    switchOn.getter = getSwitch;

    return true;
}

#endif