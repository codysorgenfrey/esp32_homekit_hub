#pragma once
#ifndef __TEMPERATURESENSORACCESSORY_H__
#define __TEMPERATURESENSORACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>

struct TempSensorAccessory : Service::TemperatureSensor {
    SpanCharacteristic *curTemp;

    TempSensorAccessory() : Service::TemperatureSensor() {
        curTemp = new Characteristic::CurrentTemperature();
    }

    boolean update() {
        return true;
    }
};

#endif