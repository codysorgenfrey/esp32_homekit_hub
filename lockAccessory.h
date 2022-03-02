#ifndef __LOCKACCESSORY_H__
#define __LOCKACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <SimpliSafe3.h>

enum HOMEKIT_LOCK_CURRENT_STATE {
    HOMEKIT_LOCK_CURRENT_STATE_UNSECURED = 0,
    HOMEKIT_LOCK_CURRENT_STATE_SECURED,
    HOMEKIT_LOCK_CURRENT_STATE_JAMMED,
    HOMEKIT_LOCK_CURRENT_STATE_UNKNOWN
};

int simpliSafeToHomekitLockSetState[2] = {
    HOMEKIT_LOCK_CURRENT_STATE_UNSECURED,
    HOMEKIT_LOCK_CURRENT_STATE_SECURED,
};

int homekitToSimpliSafeLockSetState[2] = {
    SS_SETLOCKSTATE_UNLOCK,
    SS_SETLOCKSTATE_LOCK
};

struct LockAccessory : Service::LockMechanism {
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SimpliSafe3 *ss;

    LockAccessory(SimpliSafe3 *inSS) : Service::LockMechanism() {
        curState = new Characteristic::LockCurrentState();
        tarState = new Characteristic::LockTargetState();
        ss = inSS;
    }

    boolean update() {
        boolean success = true;
        if (tarState->updated()) success &= setLockCurState();
        return success;
    }

    boolean setLockCurState() {
        HK_LOG_LINE("Setting SimpliSafe lock state.");
        int hkTarState = tarState->getNewVal();
        int res = ss->setLockState(homekitToSimpliSafeLockSetState[hkTarState]);
        if (res != SS_GETSTATE_UNKNOWN) return true;
        HK_ERROR_LINE("Error setting SimpliSafe lock state.");
        return false;
    }

    boolean getLockCurState() {
        HK_LOG_LINE("Getting SimpliSafe lock state.");
        int res = ss->getLockState();
        if (res != SS_GETLOCKSTATE_UNKNOWN) {
            curState->setVal(int(simpliSafeToHomekitLockSetState[res]));
            tarState->setVal(int(simpliSafeToHomekitLockSetState[res]));
            return true;
        }
        HK_ERROR_LINE("Error getting SimpliSafe lock state.");
        return false;
    }

    void listenToEvents(int eventId) {
        switch (eventId) {
        case 9700:
            // DOORLOCK_UNLOCKED
            HK_LOG_LINE("Got an unlock event.");
            curState->setVal(int(HOMEKIT_LOCK_CURRENT_STATE_UNSECURED));
            tarState->setVal(int(HOMEKIT_LOCK_CURRENT_STATE_UNSECURED));
            break;
        case 9701:
            // DOORLOCK_LOCKED
            HK_LOG_LINE("Got a lock event.");
            curState->setVal(int(HOMEKIT_LOCK_CURRENT_STATE_SECURED));
            tarState->setVal(int(HOMEKIT_LOCK_CURRENT_STATE_SECURED));
            break;
        case 9703:
            // DOORLOCK_ERROR
            HK_LOG_LINE("Got a door lock error event.");
            curState->setVal(int(HOMEKIT_LOCK_CURRENT_STATE_JAMMED));
            break;
        
        default:
            HK_LOG_LINE("Got an event I don't care about. %i", eventId);
            break;
        }
    }
};

#endif