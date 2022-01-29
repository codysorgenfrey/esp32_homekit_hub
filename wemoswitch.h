#ifndef __WEMOSWITCH_H__
#define __WEMOSWITCH_H__

#include "common.h"
#include <homekit/characteristics.h>
#include <ESP8266HTTPClient.h>

extern "C" homekit_characteristic_t wemo_on;
extern "C" homekit_characteristic_t wemo_name;

void setSwitch(homekit_value_t value) {
    // tell switch what homekit says
}

homekit_value_t getSwitch() {
    // tell homekit what switch says
}

bool initWemoSwitch() {
    wemo_on.setter = setSwitch;
    wemo_on.getter = getSwitch;

    return true;
}

#endif