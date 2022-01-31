#ifndef __GARAGEDOOR_H__
#define __GARAGEDOOR_H__

#include "common.h"
#include <homekit/characteristics.h>
#include <ESP8266HTTPClient.h>

extern "C" homekit_characteristic_t gdCurState;    // 0 open, 1 closed, 2 opening, 3 closing, 4 stopped 
extern "C" homekit_characteristic_t gdTarState;    // 0 open, 1 closed
extern "C" homekit_characteristic_t gdObstruction; // true, false

void setDoorState(const homekit_value_t value) {
    // tell SS what homekit says
    gdTarState.value = value;

    // set this based on what GD says in response.
    gdCurState.value = value;
}

homekit_value_t getDoorState() {
    // tell homekit what SS says
    
    

    return gdCurState.value;
}

bool initGarageDoor() {
    gdTarState.setter = setDoorState;
    gdCurState.getter = getDoorState;

    return true;
}

#endif