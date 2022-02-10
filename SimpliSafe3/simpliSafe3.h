#ifndef __SIMPLISAFE3_H__
#define __SIMPLISAFE3_H__

#include "common.h"
#include <ArduinoJson.h>
#include "ss3authManager.h"

class SimpliSafe3 {
    private:
        String subId;
        String userId;
        SS3AuthManager *authManager;

    public:
        SimpliSafe3() {
            SS_LOG_LINE("Making SS3");
            authManager = new SS3AuthManager();
        }

        bool authorize(HardwareSerial *hwSerial = &Serial, unsigned long baud = 115200) {
            if (!authManager->isAuthorized()) {
                SS_LOG_LINE("Get that damn URL code:");
                SS_LOG_LINE("%s", authManager->getSS3AuthURL().c_str());
                if (!hwSerial) hwSerial->begin(baud);
                while (hwSerial->available() > 0) { hwSerial->read(); } // flush serial monitor
                while (hwSerial->available() == 0) { delay(100); } // wait for url input
                String code = hwSerial->readString();
                hwSerial->println();
                if (authManager->getAuthToken(code)) {
                    SS_LOG_LINE("Successfully authorized Homekit with SimpliSafe.");
                    return true;
                } else { 
                    SS_LOG_LINE("Error authorizing Homekit with Simplisafe.");
                    return false;
                }
            }
                
            return authManager->refreshAuthToken();
        }

        String getUserID() {
            SS_LOG_LINE("Getting user ID");
            if (userId.length() != 0) {
                SS_LOG_LINE("Already had userID");
                return userId;
            }

            DynamicJsonDocument data = authManager->request(String(SS3API) + "/api/authCheck", 64);
            if (data.size() > 0) {
                userId = data["userId"].as<String>();

                return userId;
            }

            return "";
        }

        DynamicJsonDocument getSubscription() {
            String userIdStr = getUserID();
            if (userIdStr.length() == 0) {
                SS_LOG_LINE("Error getting userId.");
                return StaticJsonDocument<0>();
            }

            StaticJsonDocument<192> filter;
            filter[0]["sid"] = true;
            filter[0]["location"]["system"]["alarmState"] = true;
            filter[0]["location"]["system"]["isAlarming"] = true;

            SS_LOG_LINE("Getting first subscription.");
            DynamicJsonDocument sub = authManager->request(
                String(SS3API) + "/users/"+userIdStr+"/subscriptions?activeOnly=true", 
                192, 
                true,
                false,
                "",
                StaticJsonDocument<0>(),
                filter
            );

            if (sub.size() == 0) {
                SS_LOG_LINE("Error getting all subscriptions.");
                return StaticJsonDocument<0>();
            }

             // TODO: Handle other situations
            subId = String(sub[0]["sid"].as<int>());
            SS_LOG_LINE("Set subId %s.", subId.c_str());

            return sub[0];
        }

        String getAlarmState() {
            SS_LOG_LINE("Getting alarm state.");
            DynamicJsonDocument sub = getSubscription();

            if (sub.size() == 0) {
                SS_LOG_LINE("Error getting subscription.");
                return "UNKNOWN";
            }

            if (sub["location"] && sub["location"]["system"]) { 
                if (sub["location"]["system"]["isAlarming"].as<bool>())
                    return "ALARM";

                static String validStates[7] = {"OFF", "HOME", "AWAY", "AWAY_COUNT", "HOME_COUNT", "ALARM_COUNT", "ALARM"};
                String alarmState = sub["location"]["system"]["alarmState"];
                
                bool found = false;
                for (int x=0; x < 7; x++) {
                    if (alarmState.equals(validStates[x]))
                        found = true;
                }

                return found ? alarmState : "UNKNOWN";
            }
            
            SS_LOG_LINE("Subscription doesn't have location or system.");
            return "UNKNOWN";
        }

        bool setAlarmState(String newState) {
            SS_LOG_LINE("Setting alarm state");
            if (subId.length() == 0) {
                getSubscription();
            }

            DynamicJsonDocument data = authManager->request(String(SS3API) + "/ss3/subscriptions/" + subId + "/state/" + newState, 3072); // optimise size

            if (data.size() > 0) return true;

            return false;
        }
};

#endif