#include "common.h"
#include <HomeSpan.h>
#include "switchAccessory.h"
#include "securitySystemAccessory.h"

void setup()
{
    #if HK_DEBUG || SS_DEBUG
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
            new Characteristic::FirmwareRevision("0.1");
            new Characteristic::Identify();

        new Service::HAPProtocolInformation();
            new Characteristic::Version("1.1.0");

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(WM_NAME);
            new Characteristic::Manufacturer(WM_MANUFACTURER);
            new Characteristic::SerialNumber(WM_SERIALNUM);  
            new Characteristic::Model(WM_MODEL);
            new Characteristic::FirmwareRevision("0.1");
            new Characteristic::Identify();

        new WeMoSwitchAccessory();

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(SS_NAME);
            new Characteristic::Manufacturer(SS_MANUFACTURER);
            new Characteristic::SerialNumber(SS_SERIALNUM);  
            new Characteristic::Model(SS_MODEL);
            new Characteristic::FirmwareRevision("0.1");
            new Characteristic::Identify();

        new SecuritySystemAccessory();
}

void loop()
{
    homeSpan.poll();
}
