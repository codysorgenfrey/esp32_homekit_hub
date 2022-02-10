#ifndef __WEATHERAPIACCESSORY_H__
#define __WEATHERAPIACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>

struct WeatherAPIAccessory : Service::TemperatureSensor {
    SpanCharacteristic *curTemp;

    WeatherAPIAccessory() : Service::TemperatureSensor() {
        curTemp = new Characteristic::CurrentTemperature();
    }

    boolean update() {
        return true;
    }
};

#endif