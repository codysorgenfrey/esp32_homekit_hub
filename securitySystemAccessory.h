#ifndef __SECURITYSYSTEMACCESSORY_H__
#define __SECURITYSYSTEMACCESSORY_H__

#include "common.h"
#include <homekit/characteristics.h>
#include <ESP8266HTTPClient.h>
#include "SimpliSafe3/simpliSafe3.h"

SimpliSafe3 ss;

extern "C" homekit_characteristic_t ssCurState; // 0 home, 1 away, 2 night, 3 off, 4 alarm
extern "C" homekit_characteristic_t ssTarState; // 0 home, 1 away, 2 night, 3 off

void setSystemTarState(const homekit_value_t value) {
    // tell system what homekit says
    ssTarState.value = value;

    // set this based on what system says in response.
    ssCurState.value = value;
    homekit_characteristic_notify(&ssCurState, ssCurState.value);
}

homekit_value_t getSystemCurState() {
    // tell homekit what system says
    int ss3TranslatedState = 3; // default to off to be safe
    String ss3State = ss.getAlarmState();

    if (ss3State.equals("OFF")) {
        ss3TranslatedState = 3;
    } else if (ss3State.equals("HOME")) {
        ss3TranslatedState = 0;
    } else if (ss3State.equals("AWAY")) {
        ss3TranslatedState = 1;
    } else if (ss3State.equals("AWAY_COUNT")) {
        ss3TranslatedState = 1;
    } else if (ss3State.equals("HOME_COUNT")) {
        ss3TranslatedState = 0;
    } else if (ss3State.equals("ALARM_COUNT")) {
        ss3TranslatedState = 4;
    } else if (ss3State.equals("ALARM")) {
        ss3TranslatedState = 4;
    } else if (ss3State.equals("UNKOWN")) {
        HK_LOG_LINE("Unknown Simplisafe Alarm State.");
    }

    ssCurState.value.int_value = ss3TranslatedState;

    return ssCurState.value;
}

bool initSecuritySystemAccessory() {
    // login
    bool success = ss.init();
    if (success) {
        success = ss.authorize(&Serial, 115200);
    }

    // setters and getters
    ssTarState.setter = setSystemTarState;
    ssCurState.getter = getSystemCurState;

    return success;
}

void securitySystemLoop() {
    // check to refresh creds here
}

#endif