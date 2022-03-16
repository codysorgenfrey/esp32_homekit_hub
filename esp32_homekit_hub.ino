/*
TODO:
1. Inkbird: Figure out bluetooth to read sensor
2. List dependencies for project in README.md
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

WeMoSwitchAccessory *mySwitch;

SimpliSafe3 *ss;
SecuritySystemAccessory *security;
LockAccessory *lock;

MyQ *mq;
GarageDoorAccessory *door;

bool wifiConnected = false;

unsigned long lastCheck;
#define HEAP_CHECK_INT 1000 * 60 * 60 // 1 hour

void resetAPIs(const char *v) {
    HK_LOG_LINE("Reseting APIs. Option: %s", v);
    String input(v);
    input.replace("E", "");
    input.replace(" ", "");
    input.replace("\r", "");
    input.replace("\n", "");

    if (input.equals("all")) {
        HK_LOG_LINE("Reseting authorization data for all.");
        ss->setup(true);
        mq->setup(true);
    } else if (input.equals("SimpliSafe")) {
        HK_LOG_LINE("Reseting authorization data for SimpliSafe.");
        ss->setup(true);
    } else if (input.equals("MyQ")) {
        HK_LOG_LINE("Reseting authorization data for MyQ.");
        mq->setup(true);
    } else {
        HK_ERROR_LINE("Command not found: \"%s\".", v);
    }
}

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

        mySwitch = new WeMoSwitchAccessory();

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(SS_NAME);
            new Characteristic::Manufacturer(SS_MANUFACTURER);
            new Characteristic::SerialNumber(SS_SERIALNUM);  
            new Characteristic::Model(SS_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        ss = new SimpliSafe3();
        security = new SecuritySystemAccessory(ss);

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(SL_NAME);
            new Characteristic::Manufacturer(SL_MANUFACTURER);
            new Characteristic::SerialNumber(SL_SERIALNUM);  
            new Characteristic::Model(SL_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        lock = new LockAccessory(ss);

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(GD_NAME);
            new Characteristic::Manufacturer(GD_MANUFACTURER);
            new Characteristic::SerialNumber(GD_SERIALNUM);  
            new Characteristic::Model(GD_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        mq = new MyQ();
        door = new GarageDoorAccessory(mq);

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(TS_NAME);
            new Characteristic::Manufacturer(TS_MANUFACTURER);
            new Characteristic::SerialNumber(TS_SERIALNUM);  
            new Characteristic::Model(TS_MODEL);
            new Characteristic::FirmwareRevision(HK_SKETCH_VER);
            new Characteristic::Identify();

        new TempSensorAccessory();

    new SpanUserCommand('E', "<api> - Erase authorization data for linked APIs. API options are \"all\", \"SimpliSafe\", or \"MyQ\".", resetAPIs);

    homeSpan.setWifiCallback([](){
        // finish setup after wifi connects
        #if HK_DEBUG >= HK_DEBUG_LEVEL_ERROR
            sl_printf(SHEETS_URL, "Homekit Hub", "Rebooting system...\n");
        #endif

        mySwitch->startPolling();
        
        // SimpliSafe
        if (!ss->setup()) {
            HK_ERROR_LINE("Error setting up SimpliSafe API.");
        }
        if (!ss->startListeningToEvents([](int eventId) {
            security->listenToEvents(eventId);
            lock->listenToEvents(eventId);
        }, nullptr, nullptr)) {
            HK_ERROR_LINE("Error setting up event callbacks for SimpliSafe.");
        }

        if (!security->getSystemCurState()) { // set initial state
            HK_ERROR_LINE("Error getting security system initial state.");
        }
        if (!lock->getLockCurState()) { // set initial state
            HK_ERROR_LINE("Error getting lock initial state.");
        }

        // MyQ
        if (!mq->setup()) {
            HK_ERROR_LINE("Error setting up MyQ API.");
        }
        door->startPolling();

        lastCheck = millis() + HEAP_CHECK_INT + (60000 * 2); // trigger heap check in x minutes
        wifiConnected = true;
    });
}

void loop() {
    unsigned long now = millis();
    unsigned long diff = max(now, lastCheck) - min(now, lastCheck);
    if (diff >= HEAP_CHECK_INT) {
        sl_printf(SHEETS_URL, "Homekit Hub", "Heap size: %.2fkb\n", (esp_get_free_heap_size() * 0.001f));
        lastCheck = now;
    }

    homeSpan.poll();
    if (wifiConnected) {
        ss->loop();
        mq->loop();
    }
}
