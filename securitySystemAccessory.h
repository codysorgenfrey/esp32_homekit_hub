#ifndef __SECURITYSYSTEMACCESSORY_H__
#define __SECURITYSYSTEMACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <SimpliSafe3.h>

struct SecuritySystemAccessory : Service::SecuritySystem {
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SimpliSafe3 *ss;
    bool ssAuthorized = false;

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
    
    void getSystemCurState() {
        // tell homekit what system says
        int ssTranslatedState = 3; // default to off to be safe
        int ssState = ss->getAlarmState();

        if (ssState == SS_GETSTATE_OFF) {
            ssTranslatedState = 3;
        } else if (ssState == SS_GETSTATE_HOME) {
            ssTranslatedState = 0;
        } else if (ssState == SS_GETSTATE_AWAY) {
            ssTranslatedState = 1;
        } else if (ssState == SS_GETSTATE_AWAY_COUNT) {
            ssTranslatedState = 1;
        } else if (ssState == SS_GETSTATE_HOME_COUNT) {
            ssTranslatedState = 0;
        } else if (ssState == SS_GETSTATE_ALARM_COUNT) {
            ssTranslatedState = 4;
        } else if (ssState == SS_GETSTATE_ALARM) {
            ssTranslatedState = 4;
        } else if (ssState == SS_GETSTATE_UNKNOWN) {
            HK_LOG_LINE("Unknown Simplisafe Alarm State.");
        }

        curState->setVal(ssTranslatedState);
    }

    boolean setSystemCurState() {
        boolean success = true;
        return success;
    }
};

#endif