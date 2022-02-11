#ifndef __SECURITYSYSTEMACCESSORY_H__
#define __SECURITYSYSTEMACCESSORY_H__

#include "common.h"
#include <HomeSpan.h>
#include <WiFi.h>
#include "SimpliSafe3/simpliSafe3.h"

struct SecuritySystemAccessory : Service::SecuritySystem {
    SpanCharacteristic *curState;
    SpanCharacteristic *tarState;
    SimpliSafe3 *ss;
    bool ssAuthorized = false;

    SecuritySystemAccessory() : Service::SecuritySystem() {
        curState = new Characteristic::SecuritySystemCurrentState(); // 0 home, 1 away, 2 night, 3 off, 4 alarm
        tarState = new Characteristic::SecuritySystemTargetState(); // 0 home, 1 away, 2 night, 3 off
        ss = new SimpliSafe3();
    }

    boolean update() {
        boolean success = true;
        if (tarState->updated()) success = setSystemCurState();

        return success;
    }

    void loop() {
        if (WiFi.status() == WL_CONNECTED && !ssAuthorized) {
            ssAuthorized = ss->authorize();
            getSystemCurState();
        }

        if (curState->timeVal() >= SS_UPDATE_INTERVAL) getSystemCurState();
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
        int hkState = tarState->getNewVal();
        SS_SETSTATE ssState;

        if (hkState == 0) {
            ssState = SS_SETSTATE_HOME;
        } else if (hkState == 1) {
            ssState = SS_SETSTATE_AWAY;
        } else if (hkState == 2) {
            ssState = SS_SETSTATE_HOME;
        } else if (hkState == 3) {
            ssState = SS_SETSTATE_OFF;
        }

        return ss->setAlarmState(ssState);
    }
};

// SimpliSafe3 ss;

// extern "C" homekit_characteristic_t ssCurState;
// extern "C" homekit_characteristic_t ssTarState;

// void setSystemTarState(const homekit_value_t value) {
//     // tell system what homekit says
//     ssTarState.value = value;

//     // set this based on what system says in response.
//     ssCurState.value = value;
//     homekit_characteristic_notify(&ssCurState, ssCurState.value);
// }

// homekit_value_t getSystemCurState() {
//     // tell homekit what system says
//     int ss3TranslatedState = 3; // default to off to be safe
//     String ss3State = ss.getAlarmState();

//     if (ss3State.equals("OFF")) {
//         ss3TranslatedState = 3;
//     } else if (ss3State.equals("HOME")) {
//         ss3TranslatedState = 0;
//     } else if (ss3State.equals("AWAY")) {
//         ss3TranslatedState = 1;
//     } else if (ss3State.equals("AWAY_COUNT")) {
//         ss3TranslatedState = 1;
//     } else if (ss3State.equals("HOME_COUNT")) {
//         ss3TranslatedState = 0;
//     } else if (ss3State.equals("ALARM_COUNT")) {
//         ss3TranslatedState = 4;
//     } else if (ss3State.equals("ALARM")) {
//         ss3TranslatedState = 4;
//     } else if (ss3State.equals("UNKOWN")) {
//         HK_LOG_LINE("Unknown Simplisafe Alarm State.");
//     }

//     ssCurState.value.int_value = ss3TranslatedState;

//     return ssCurState.value;
// }

// bool initSecuritySystemAccessory() {
//     // login
//     bool success = ss.init();
//     if (success) {
//         success = ss.authorize(&Serial, 115200);
//     }

//     // setters and getters
//     ssTarState.setter = setSystemTarState;
//     ssCurState.getter = getSystemCurState;

//     return success;
// }

// void securitySystemLoop() {
//     // check to refresh creds here
// }

#endif