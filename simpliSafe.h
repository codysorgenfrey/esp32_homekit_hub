#ifndef __SIMPLISAFE_H__
#define __SIMPLISAFE_H__

#include "common.h"
#include <homekit/characteristics.h>
#include <ESP8266HTTPClient.h>

extern "C" homekit_characteristic_t ssCurState; // 0 home, 1 away, 2 night, 3 off, 4 alarm
extern "C" homekit_characteristic_t ssTarState; // 0 home, 1 away, 2 night, 3 off

void setSystem(const homekit_value_t value) {
    // tell SS what homekit says
    ssTarState.value = value;

    // set this based on what SS says in response.
    ssCurState.value = value;
}

homekit_value_t getSystem() {
    // tell homekit what SS says

    return ssCurState.value;
}

bool initSimpliSafe() {
    ssTarState.setter = setSystem;
    ssCurState.getter = getSystem;

    return true;
}

#endif