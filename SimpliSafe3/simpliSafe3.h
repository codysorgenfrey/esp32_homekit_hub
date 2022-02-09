#ifndef __SIMPLISAFE3_H__
#define __SIMPLISAFE3_H__

#include "ssCommon.h"
#include <ArduinoJson.h>
#include "ss3authManager->h"

class SimpliSafe3 {
    private:
        String *subId;
        String *userId;
        SS3AuthManager *authManager;

    public:
        SimpliSafe3() {
            SS_LOG_LINE("Making SS3");
            subId = new String();
            userId = new String();
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

        String* getUserID() {
            if (userId->length() != 0) {
                return userId;
            }

            DynamicJsonDocument data = request("/api/authCheck", false);
            userId = data["userId"].as<String>();

            return userId;
        }

        DynamicJsonDocument request(String path, bool post = false, String payload = "", int docSize = 3072) {
            StaticJsonDocument<256> headers;
            headers["Authorization"] = authManager->tokenType + " " + authManager->accessToken;

            return authManager->request(SS3API + path, post, payload, headers, docSize);
        }

        DynamicJsonDocument getSubscriptions() {
            String userID = getUserID();
            DynamicJsonDocument subscriptions = request("/users/"+userID+"/subscriptions?activeOnly=false", false);

            return subscriptions;
        }

        DynamicJsonDocument getSubscription() {
            DynamicJsonDocument subs = getSubscriptions();

            if (!subId && subs.size() == 1) { // TODO: Handle other situations
                subId = subs[0]["sid"].as<String>();
            }

            return subs[0];
        }

        String getAlarmState() {
            DynamicJsonDocument subscription = getSubscription();

            if (subscription["location"] && subscription["location"]["system"]) { 
                if (subscription["location"]["system"]["isAlarming"].as<bool>())
                    return "ALARM";

                static String validStates[7] = {"OFF", "HOME", "AWAY", "AWAY_COUNT", "HOME_COUNT", "ALARM_COUNT", "ALARM"};
                String alarmState = subscription["location"]["system"]["alarmState"];
                
                bool found = false;
                for (int x=0; x < 7; x++) {
                    if (alarmState.equals(validStates[x]))
                        found = true;
                }

                return found ? alarmState : "UNKOWN";
            } else {
                return "UNKOWN";
            }
        }

        bool setAlarmState(String newState) {
            if (subId->length() == 0) {
                getSubscription();
            }

            DynamicJsonDocument data = request(
                "/ss3/subscriptions/" + subId + "/state/" + newState, 
                true
            );

            // check if it worked here

            return true;
        }
};

#endif