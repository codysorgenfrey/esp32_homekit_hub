#pragma once
#ifndef __TEMPERATURESENSORACCESSORY_H__
#define __TEMPERATURESENSORACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))
float inkbirdTemp;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getAddress().toString() == INKBIRD_BLE_ADDRESS) {
            HK_LOG_LINE("Found Inkbird!");

            std::string manData = advertisedDevice.getManufacturerData();
            const char *hexData = BLEUtils::buildHexData(nullptr, (uint8_t *)manData.data(), manData.length());

            std::string hexString(hexData);
            const char *tempString = hexString.substr(0, 4).c_str();

            unsigned long tempCelc = ENDIAN_CHANGE_U16(strtoul(tempString, NULL, 16));
            inkbirdTemp = (tempCelc / 100.0f) * 1.8f + 32.0f;

            HK_LOG_LINE("Inkbird temp: %dF", inkbirdTemp);
        }
    }
};

struct TempSensorAccessory : Service::TemperatureSensor {
    SpanCharacteristic *curTemp;
    unsigned long lastPoll; // we have to track polling ourselves since we might not update each poll

    TempSensorAccessory() : Service::TemperatureSensor() {
        curTemp = new Characteristic::CurrentTemperature();
        scanAndUpdateTemp(); // not sure if we need to do this right away.
    }

    boolean update() {
        return true;
    }

    void loop() {
        unsigned long now = millis();
        unsigned long timeDiff = max(now, lastPoll) - min(now, lastPoll);
        if (timeDiff >= TS_UPDATE_INTERVAL) scanAndUpdateTemp();
    }

    void scanAndUpdateTemp() {
        HK_LOG_LINE("Starting Inkbird Scan.");
        int scanTime = 1;
        BLEScan *pBLEScan;
        BLEDevice::init("");
        pBLEScan = BLEDevice::getScan(); // create new scan
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster
        pBLEScan->setInterval(scanTime * 1000);
        pBLEScan->setWindow((scanTime * 1000) - 1); // less or equal setInterval value

        pBLEScan->start(scanTime, false);
        HK_LOG_LINE("Scan done!");
        pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

        // Update homekit
        curTemp->setVal(inkbirdTemp);
    }
};

#endif