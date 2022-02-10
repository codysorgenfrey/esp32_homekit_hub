#ifndef __GARAGEDOORACCESSORY_H__
#define __GARAGEDOORACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>

struct GarageDoorAccessory : Service::GarageDoorOpener {
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SpanCharacteristic *obstructed;

    GarageDoorAccessory() : Service::GarageDoorOpener() {
        curState = new Characteristic::CurrentDoorState();
        tarState = new Characteristic::TargetDoorState();
        obstructed = new Characteristic::ObstructionDetected();
    }

    boolean update() {
        return updateSimpliSafeState();
    }

    void loop() {

    }

    boolean updateSimpliSafeState() {
        return true;
    }

    void updateHomekitState() {
        
    }
};

#endif

// #ifndef __GARAGEDOOR_H__
// #define __GARAGEDOOR_H__

// #include "common.h"

// extern "C" homekit_characteristic_t gdCurState;    // 0 open, 1 closed, 2 opening, 3 closing, 4 stopped 
// extern "C" homekit_characteristic_t gdTarState;    // 0 open, 1 closed
// extern "C" homekit_characteristic_t gdObstruction; // true, false

// void setDoorTarState(const homekit_value_t value) {
//     // tell GD what homekit says
//     gdTarState.value = value;

//     // set this based on what GD says in response.
//     gdCurState.value = value;
// }

// homekit_value_t getDoorCurState() {
//     // tell homekit what GD says

//     return gdCurState.value;
// }

// bool initGarageDoorAccessory() {
//     gdTarState.setter = setDoorTarState;
//     gdCurState.getter = getDoorCurState;

//     return true;
// }

// #endif