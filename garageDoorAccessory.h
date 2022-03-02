#ifndef __GARAGEDOORACCESSORY_H__
#define __GARAGEDOORACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>

// cur state 0 = open, 1 = closed, 2 = opening, 3 = closing, 4 = stopped
// tar state 0 = open, 1 = closed

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
        if (tarState->updated()) {
            delay(1000);
            curState->setVal(tarState->getNewVal());
        }
    }
};

#endif