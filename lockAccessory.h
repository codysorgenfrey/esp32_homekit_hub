#ifndef __LOCKACCESSORY_H__
#define __LOCKACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>

struct LockAccessory : Service::LockMechanism {
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;

    LockAccessory() : Service::LockMechanism() {
        curState = new Characteristic::LockCurrentState();
        tarState = new Characteristic::LockTargetState();
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