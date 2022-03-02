#ifndef __GARAGEDOORACCESSORY_H__
#define __GARAGEDOORACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <MyQ.h>

enum HOMEKIT_GARAGE_DOOR_CURRENT_STATE {
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_OPEN = 0,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_CLOSED,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_OPENING,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_CLOSING,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_STOPPED
};

enum HOMEKIT_GARAGE_DOOR_TARGET_STATE {
    HOMEKIT_GARAGE_DOOR_TARGET_STATE_OPEN = 0,
    HOMEKIT_GARAGE_DOOR_TARGET_STATE_CLOSED,
};

int myQToHomekitState[6] = {
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_CLOSED,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_CLOSING,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_OPEN,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_OPENING,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_STOPPED,
    HOMEKIT_GARAGE_DOOR_CURRENT_STATE_OPENING
};

MYQ_DOOR_SETSTATE homekitTargetStateToMyQ[2] = {
    MYQ_DOOR_SETSTATE_OPEN,
    MYQ_DOOR_SETSTATE_CLOSE
};

struct GarageDoorAccessory : Service::GarageDoorOpener {
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SpanCharacteristic *obstructed;
    MyQ *mq;
    bool polling = false;
    unsigned long lastPoll; // we have to track polling ourselves since we might not update each poll
    bool active = false;

    GarageDoorAccessory(MyQ *inMq) : Service::GarageDoorOpener() {
        curState = new Characteristic::CurrentDoorState();
        tarState = new Characteristic::TargetDoorState();
        obstructed = new Characteristic::ObstructionDetected();
        mq = inMq;
    }

    boolean update() {
        boolean success = true;
        if (tarState->updated()) success &= setDoorState();
        return success; 
    }

    void loop() {
        if (polling) {
            unsigned long now = millis();
            unsigned long timeDiff = max(now, lastPoll) - min(now, lastPoll);
            if (
                (active && timeDiff >= GD_ACTIVE_UPDATE_INTERVAL) ||
                (timeDiff >= GD_UPDATE_INTERVAL)
            ) {
                pollDoorState();
                if (curState->timeVal() >= GD_ACTIVE_UPDATE_DURATION) active = false;
            }
        }
    }

    void startPolling() {
        HK_LOG_LINE("Starting poll for MyQ door state.");
        polling = true;
        pollDoorState();
    }

    void pollDoorState() {
        int res = mq->getGarageState(GD_SERIALNUM);
        
        if (res != MYQ_DOOR_GETSTATE_UNKNOWN) {
            lastPoll = millis();
            int myQCurState = myQToHomekitState[res];
            if (myQCurState != curState->getVal()) {
                HK_LOG_LINE("Found updated garage door state.");
                curState->setVal(myQCurState);
                if (
                    myQCurState == HOMEKIT_GARAGE_DOOR_CURRENT_STATE_CLOSED ||
                    myQCurState == HOMEKIT_GARAGE_DOOR_CURRENT_STATE_OPEN
                ) {
                    tarState->setVal(myQCurState);
                }
            }
        } else {
            HK_ERROR_LINE("Error polling MyQ door state.");
        }
    }

    boolean setDoorState() {
        HK_LOG_LINE("Setting MyQ door state.");
        int hkTarState = tarState->getNewVal();
        int res = mq->setGarageState(GD_SERIALNUM, homekitTargetStateToMyQ[hkTarState]);

        if (res != MYQ_DOOR_GETSTATE_UNKNOWN) {
            active = true;
            return true;
        }

        HK_ERROR_LINE("Unable to update MyQ door state.");
        return false;
    }
};

#endif