#include "common.h"
#include <arduino_homekit_server.h> // need to disable logging in homekit_debug.h
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include "switchAccessory.h"
#include "securitySystemAccessory.h"
#include "garageDoorAccessory.h"

extern "C" homekit_server_config_t config;

unsigned long int boardStatus = STATUS_NO_WIFI;

int statusBlinkPattern[7] = {
    10, // Error
    0,  // ok
    1,  // no wifi
    2,  // No OTA
    3,  // No Homkit
    3   // OTA progress
};
int statusPatternRate[7] = {
    2000, // error
    1000, // ok
    1000, // no wifi
    1000, // No OTA
    1000, // No homekit
    2000  // OTA progress
};
unsigned long lastBlinkMs = 0;

void handleStatus() {
    unsigned long nowMs = millis();
    int diff = nowMs - lastBlinkMs;
    if (abs(diff) >= statusPatternRate[boardStatus]) {
        if (boardStatus == STATUS_OK && digitalRead(STATUS_LED) == LED_ON) {
            digitalWrite(STATUS_LED, LED_OFF);
        } else {
            for (int x = 0; x < statusBlinkPattern[boardStatus]; x++) {
                digitalWrite(STATUS_LED, LED_ON);
                delay(99);
                digitalWrite(STATUS_LED, LED_OFF);
                delay(99);
            }
        }

        lastBlinkMs = nowMs;
    }
}

void setup()
{
    #if HK_DEBUG || SS_DEBUG
        Serial.begin(115200);
        while (!Serial) { ; }; // wait for serial
    #endif
    HK_LOG_LINE("Starting...");

    // Setup output LED
    pinMode(STATUS_LED, OUTPUT); // set up status LED
    pinMode(PWR_LED, OUTPUT);    // set up power LED
    digitalWrite(STATUS_LED, LED_OFF);
    digitalWrite(PWR_LED, LED_ON);

    if (boardStatus == STATUS_NO_WIFI) {
        // Connect to wifi
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
    
    if (boardStatus == STATUS_NO_OTA) {
        // Connect OTA
        ArduinoOTA.setHostname(HK_UNIQUE_NAME);
        ArduinoOTA.setPassword(OTA_PASS);
        ArduinoOTA.setRebootOnSuccess(true);
        ArduinoOTA.onStart([](){ 
            boardStatus = STATUS_OTA_PROGRESS;
            HK_LOG_LINE("Starting OTA Update");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { handleStatus(); });
        ArduinoOTA.onError([](ota_error_t error) { 
            boardStatus = STATUS_ERROR;
            HK_LOG("OTA Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) { HK_LOG_LINE("Auth Failed"); }
            else if (error == OTA_BEGIN_ERROR) { HK_LOG_LINE("Begin Failed"); }
            else if (error == OTA_CONNECT_ERROR) { HK_LOG_LINE("Connect Failed"); }
            else if (error == OTA_RECEIVE_ERROR) { HK_LOG_LINE("Receive Failed"); }
            else if (error == OTA_END_ERROR) { HK_LOG_LINE("End Failed"); }
        });
        ArduinoOTA.begin();

        boardStatus = STATUS_NO_HOMEKIT;
    }
        
    if (boardStatus == STATUS_NO_HOMEKIT) {
        // init accessories
        boardStatus = initSwitchAccessory() ? STATUS_NO_HOMEKIT : STATUS_ERROR;

        if (boardStatus != STATUS_ERROR)
            boardStatus = initSecuritySystemAccessory() ? STATUS_NO_HOMEKIT : STATUS_ERROR;

        if (boardStatus != STATUS_ERROR)
            boardStatus = initGarageDoorAccessory() ? STATUS_NO_HOMEKIT : STATUS_ERROR;

        // Connect to Homekit
        if (boardStatus != STATUS_ERROR) {
            homekit_server_reset();
            arduino_homekit_setup(&config);
            boardStatus = STATUS_OK;
        }
    }
}

void loop()
{
    // Handle OTA 
    if (boardStatus != STATUS_NO_OTA) ArduinoOTA.handle();
    
    // Status blinks
    handleStatus();

    // Handle WiFi
    if (boardStatus == STATUS_NO_WIFI) {
        delay(100);
        if (WiFi.status() == WL_CONNECTED) {
            boardStatus = STATUS_NO_OTA;
            setup();
        }
        return;
    }

    // Handle Homekit
    if (boardStatus != STATUS_NO_HOMEKIT) arduino_homekit_loop();

    if (boardStatus != STATUS_ERROR) securitySystemLoop();
}
