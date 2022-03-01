/*
TODO:
1. Weather API: Get weather
2. Inkbird: Figure out bluetooth to read sensor
3. HK: Hook up accessories
4. List dependencies for project in README.md
*/

#include "common.h"
#include <HomeSpan.h>
#include <SimpliSafe3.h>
#include <MyQ.h>
#include "switchAccessory.h"
#include "securitySystemAccessory.h"
#include "lockAccessory.h"
#include "garageDoorAccessory.h"
#include "tempSensorAccessory.h"
#include "weatherAPIAccessory.h"

SimpliSafe3 ss;
MyQ mq;

void setup()
{
    #if HK_DEBUG
        Serial.begin(115200);
        while (!Serial) { ; }; // wait for serial
    #endif
    HK_LOG_LINE("Starting...");

    homeSpan.enableOTA();
    homeSpan.setStatusPin(STATUS_LED);
    homeSpan.setSketchVersion(HK_SKETCH_VER);
    homeSpan.begin(Category::Bridges, HK_UNIQUE_NAME, HK_MANUFACTURER, HK_MODEL);

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(HK_NAME);
            new Characteristic::Manufacturer(HK_MANUFACTURER);
            new Characteristic::SerialNumber(HK_SERIALNUM);  
            new Characteristic::Model(HK_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        new Service::HAPProtocolInformation();
            new Characteristic::Version("1.1.0");

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(WM_NAME);
            new Characteristic::Manufacturer(WM_MANUFACTURER);
            new Characteristic::SerialNumber(WM_SERIALNUM);  
            new Characteristic::Model(WM_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        new WeMoSwitchAccessory();

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(SS_NAME);
            new Characteristic::Manufacturer(SS_MANUFACTURER);
            new Characteristic::SerialNumber(SS_SERIALNUM);  
            new Characteristic::Model(SS_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        new SecuritySystemAccessory(&ss);

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(SL_NAME);
            new Characteristic::Manufacturer(SL_MANUFACTURER);
            new Characteristic::SerialNumber(SL_SERIALNUM);  
            new Characteristic::Model(SL_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        new LockAccessory();

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(GD_NAME);
            new Characteristic::Manufacturer(GD_MANUFACTURER);
            new Characteristic::SerialNumber(GD_SERIALNUM);  
            new Characteristic::Model(GD_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        new GarageDoorAccessory();

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(TS_NAME);
            new Characteristic::Manufacturer(TS_MANUFACTURER);
            new Characteristic::SerialNumber(TS_SERIALNUM);  
            new Characteristic::Model(TS_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        new TempSensorAccessory();

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(WA_NAME);
            new Characteristic::Manufacturer(WA_MANUFACTURER);
            new Characteristic::SerialNumber(WA_SERIALNUM);  
            new Characteristic::Model(WA_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        new WeatherAPIAccessory();
}

void loop()
{
    homeSpan.poll();
}
