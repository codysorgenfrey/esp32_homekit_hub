#ifndef __SIMPLISAFE3_H__
#define __SIMPLISAFE3_H__

#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "ss3AuthManager.h"

#define ss3Api "https://api.simplisafe.com/v1"

class SimpliSafe3 {
    private:
        String subId;
        String userId;
        bool logging;
        SS3AuthManager *authManager;

    public:
        SimpliSafe3(SS3AuthManager *inAuthManager, bool inLogging = true) {
            this->authManager = inAuthManager;
            this->logging = inLogging;
        }

        String getUserID() {
            if (this->userId.length() != 0) {
                return this->userId;
            }

            DynamicJsonDocument data = this->request("/api/authCheck", false);
            this->userId = data["userId"].as<String>();

            return this->userId;
        }

        DynamicJsonDocument request(String path, bool post = false, String payload = "") {
            WiFiClient client;
            HTTPClient http;
            http.useHTTP10(true); // need to use for ArduinoJson

            http.begin(client, ss3Api + path);
            http.addHeader("Authorization", this->authManager->tokenType + " " + this->authManager->accessToken);
            int responseCode;

            if (post)
                responseCode = http.POST(payload);
            else
                responseCode = http.GET();

            if (responseCode >= 200 && responseCode <= 299) { // OK
                DynamicJsonDocument doc(256); // TODO: optimize size
                DeserializationError err = deserializeJson(doc, http.getStream());

                if (this->logging && err) {
                    Serial.println(F("SS3 Error: API request"));
                    Serial.println(err.f_str());
                }

                return doc;
            } else if (this->logging) {
                Serial.print(F("SS3 Error: API response: "));
                Serial.println(responseCode);
            }

            client.stop();
            http.end();
            
            return StaticJsonDocument<0>();
        }

        DynamicJsonDocument getSubscriptions() {
            String userID = this->getUserID();
            DynamicJsonDocument subscriptions = this->request("/users/"+userID+"/subscriptions?activeOnly=false", false);

            return subscriptions;
        }

        DynamicJsonDocument getSubscription() {
            DynamicJsonDocument subs = this->getSubscriptions();

            if (!this->subId && subs.size() == 1) { // TODO: Handle other situations
                this->subId = subs[0]["sid"].as<String>();
            }

            return subs[0];
        }

        String getAlarmState() {
            DynamicJsonDocument subscription = this->getSubscription();

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
            if (this->subId.length() == 0) {
                this->getSubscription();
            }

            DynamicJsonDocument data = this->request(
                "/ss3/subscriptions/" + this->subId + "/state/" + newState, 
                true
            );

            // check if it worked here

            return true;
        }
};

#endif