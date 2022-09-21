/*
TODO:
1. List dependencies for project in README.md
*/

#include "common.h"
#include <HomeSpan.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
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

TempSensorAccessory *tempSensor;

WebSocketsServer webSocket = WebSocketsServer(81);

bool wifiConnected = false;

unsigned long lastCheck;

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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            HK_LOG_LINE("#%u Disconnected\n", num);
            break;
        case WStype_CONNECTED:
        {
            IPAddress ip = webSocket.remoteIP(num);
            HK_LOG_LINE("#%u Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
        break;
        case WStype_TEXT: {
            HK_LOG_LINE("[%u] get Text: %s\n", num, payload);
            

            if (String((char *)payload) != String("Connected")) {
                StaticJsonDocument<256> doc; 
                DeserializationError err = deserializeJson(doc, payload);

                if (err) 
                    HK_ERROR_LINE("Error deserializing json from websocket event. Payload: %s", payload);
                else {
                    String device = doc["device"].as<String>();
                    if (device == String("IBS-TH2")) {
                        if (tempSensor->handleMessage(doc)) {
                            webSocket.sendTXT(num, "Success");
                        } else {
                            webSocket.sendTXT(num, "Error");
                        }
                    }
                }

                doc.clear();
            }

            break;
        }
        case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}

void setup()
{
    #if HK_DEBUG > HK_DEBUG_LEVEL_NONE
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

        tempSensor = new TempSensorAccessory();

    new SpanUserCommand('E', "<api> - Erase authorization data for linked APIs. API options are \"all\", \"SimpliSafe\", or \"MyQ\".", resetAPIs);

    homeSpan.setWifiCallback([](){
        // finish setup after wifi connects
        #if HK_DEBUG > HK_DEBUG_LEVEL_NONE
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

        webSocket.setAuthorization(WEBSOCKET_USER, WEBSOCKET_PASS);
        webSocket.begin();
        webSocket.onEvent(webSocketEvent);

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
        webSocket.loop();
    }
}
