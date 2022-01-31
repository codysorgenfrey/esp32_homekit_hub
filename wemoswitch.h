#ifndef __WEMOSWITCH_H__
#define __WEMOSWITCH_H__

#include "common.h"
#include <homekit/characteristics.h>
#include <ESP8266HTTPClient.h>

extern "C" homekit_characteristic_t wemoOn;

void setSwitch(const homekit_value_t value) {
    // tell switch what homekit says

    // Start First HTTP POST to retrieve switch status (0 for off, 1 for on)
    String stringState = value.bool_value ? "1" : "0";
    String postData = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>" + stringState + "</BinaryState></u:SetBinaryState></s:Body></s:Envelope>";
    
    WiFiClient client;
    HTTPClient http;
    http.begin(client, (String) "http://" + WEMO_IP + ":49153/upnp/control/basicevent1");
    http.addHeader("Content-Length", (String)postData.length());
    http.addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
    http.addHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#SetBinaryState\"");
    
    #if HK_DEBUG
        Serial.println("Sending POST request to Wemo.");
    #endif
    int httpCode = http.POST(postData);

    // If HTTP code is not negative, the POST succeeded
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            #if HK_DEBUG
                Serial.println("Wemo switch on.");
            #endif
            wemoOn.value = value;
        } else {
            #if HK_DEBUG
                Serial.println("Switch responded not ok.");
            #endif
        }
    } else {
        #if HK_DEBUG
            Serial.println("POST request error.");
        #endif
    }

    http.end(); // these might kill the WiFi... we'll see
    client.stop();
}

homekit_value_t getSwitch() {
    // tell homekit what switch says
    
    // Start First HTTP POST to retrieve switch status (0 for off, 1 for on)
    String postData = "<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"></u:GetBinaryState></s:Body></s:Envelope>";
    
    WiFiClient client;
    HTTPClient http;
    http.begin(client, (String) "http://" + WEMO_IP + ":49153/upnp/control/basicevent1");
    http.addHeader("Content-Length", (String)postData.length());
    http.addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
    http.addHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#GetBinaryState\"");
    
    #if HK_DEBUG
        Serial.println("Sending POST request to Wemo.");
    #endif
    int httpCode = http.POST(postData);

    // If HTTP code is not negative, the POST succeeded
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            const String &payload = http.getString();
            if (payload.indexOf("<BinaryState>1</BinaryState>") > 0) {
                #if HK_DEBUG
                    Serial.println("Wemo switch on.");
                #endif
                wemoOn.value.bool_value = true;
            } else {
                #if HK_DEBUG
                    Serial.println("Wemo switch off.");
                #endif
                wemoOn.value.bool_value = false;
            }
        } else {
            #if HK_DEBUG
                Serial.println("Switch responded not ok.");
            #endif
        }
    } else {
        #if HK_DEBUG
            Serial.println("POST request error.");
        #endif
    }

    http.end(); // these might kill the WiFi... we'll see
    client.stop();

    return wemoOn.value;
}

bool initWemoSwitch() {
    wemoOn.setter = setSwitch;
    wemoOn.getter = getSwitch;

    return true;
}

#endif