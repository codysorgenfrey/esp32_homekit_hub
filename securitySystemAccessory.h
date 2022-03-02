#ifndef __SECURITYSYSTEMACCESSORY_H__
#define __SECURITYSYSTEMACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <SimpliSafe3.h>

enum HOMEKIT_SECURITYSYSTEM_CURRENTSTATE {
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_HOME = 0,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_AWAY,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_NIGHT,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_OFF,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_ALARM,
};

enum HOMEKIT_SECURITYSYSTEM_TARGETSTATE {
    HOMEKIT_SECURITYSYSTEM_TARGETSTATE_HOME = 0,
    HOMEKIT_SECURITYSYSTEM_TARGETSTATE_AWAY,
    HOMEKIT_SECURITYSYSTEM_TARGETSTATE_NIGHT,
    HOMEKIT_SECURITYSYSTEM_TARGETSTATE_OFF,
};

int homekitToSimpliSafeSetState[4] = {
    SS_SETSTATE_HOME,
    SS_SETSTATE_AWAY,
    SS_SETSTATE_HOME,
    SS_SETSTATE_OFF
};

int SimpliSafeToHomekitSetState[7] = {
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_OFF,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_HOME,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_HOME,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_AWAY,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_AWAY,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_ALARM,
    HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_ALARM,
};

struct SecuritySystemAccessory : Service::SecuritySystem {
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SimpliSafe3 *ss;

    SecuritySystemAccessory(SimpliSafe3 *inSS) : Service::SecuritySystem() {
        curState = new Characteristic::SecuritySystemCurrentState(); // 0 home, 1 away, 2 night, 3 off, 4 alarm
        tarState = new Characteristic::SecuritySystemTargetState(); // 0 home, 1 away, 2 night, 3 off
        ss = inSS;
    }

    boolean update() {
        boolean success = true;
        if (tarState->updated()) success = setSystemCurState();

        return success;
    }

    boolean setSystemCurState() {
        HK_LOG_LINE("Setting SimpliSafe alarm state.");
        int hkTarState = tarState->getNewVal();
        int res = ss->setAlarmState(homekitToSimpliSafeSetState[hkTarState]);
        if (res != SS_GETSTATE_UNKNOWN) return true;
        HK_ERROR_LINE("Error setting SimpliSafe alarm state.");
        return false;
    }

    boolean getSystemCurState() {
        HK_LOG_LINE("Getting SimpliSafe alarm state.");
        int res = ss->getAlarmState();
        if (res != SS_GETSTATE_UNKNOWN) {
            if (
                res != SS_GETSTATE_HOME_COUNT &&
                res != SS_GETSTATE_AWAY_COUNT &&
                res != SS_GETSTATE_ALARM_COUNT
            ) {
                curState->setVal(int(SimpliSafeToHomekitSetState[res]));
            }
            
            if (
                res != SS_GETSTATE_ALARM && 
                res != SS_GETSTATE_ALARM_COUNT
            ) {
                tarState->setVal(int(SimpliSafeToHomekitSetState[res]));
            }

            return true;
        }
        HK_ERROR_LINE("Error getting SimpliSafe alarm state.");
        return false;
    }

    void listenToEvents(int eventId) {
        switch (eventId) {
        case 1110:
        case 1120:
        case 1132:
        case 1134:
        case 1154:
        case 1159:
        case 1162:
            // Alarming
            HK_LOG_LINE("Got an alarm event.");
            curState->setVal(int(HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_ALARM));
            break;

        case 1400:
        case 1407:
        case 1406:
            // Disarm
            HK_LOG_LINE("Got a disarm event.");
            curState->setVal(int(HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_OFF));
            break;

        case 3401:
        case 3407:
        case 3487:
        case 3481:
            // Away arm
            HK_LOG_LINE("Got an away arm event.");
            curState->setVal(int(HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_AWAY));
            break;

        case 3441:
        case 3491:
            // Home arm
            HK_LOG_LINE("Got a home arm event.");
            curState->setVal(int(HOMEKIT_SECURITYSYSTEM_CURRENTSTATE_HOME));
            break;

        case 9401:
        case 9407:
            // away exit delay
            HK_LOG_LINE("Got an away exit delay event.");
            tarState->setVal(int(HOMEKIT_SECURITYSYSTEM_TARGETSTATE_AWAY));
            break;

        case 9441:
            // home exit delay
            HK_LOG_LINE("Got a home exit delay event.");
            tarState->setVal(int(HOMEKIT_SECURITYSYSTEM_TARGETSTATE_HOME));
            break;
        
        default:
            HK_LOG_LINE("Got an event. %i", eventId);
            break;
        }
    }
};

#endif