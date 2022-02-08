#ifndef __SS3AUTHMANAGER_H__
#define __SS3AUTHMANAGER_H__

// TODO: store account in eeprom

#include "ssCommon.h"
#include <ESP8266HTTPclient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <crypto.h>
#include <SHA256.h>

#define SHA256_LEN 32

class SS3AuthManager {
    private:
        String refreshToken;
        String codeVerifier;
        String codeChallenge;
        unsigned long tokenIssueMS;
        unsigned long expiresIn;

    public:
        HTTPClient *https;
        WiFiClientSecure *client;
        String tokenType = "Bearer";
        String accessToken;

        bool init(HTTPClient *inHttps, WiFiClientSecure *inClient) {
            // init https stuff
            if (inHttps && inClient) {
                SS_LOG_LINE("Provided https and client");
                https = inHttps;
                client = inClient;
            } else {
                SS_LOG_LINE("No https setup.");
                return false;
            }
            
            https->useHTTP10(true); // for ArduinoJson
            https->setReuse(false); // to reuse connection

            if(!readUserData()) {
                uint8_t randData[32]; // 32 bytes, u_int8_t is 1 byte
                ESP.random(randData, SHA256_LEN);
                codeVerifier = base64URLEncode(randData);

                uint8_t hashOut[SHA256_LEN];
                sha256(codeVerifier.c_str(), hashOut);
                codeChallenge = base64URLEncode(hashOut);
            }

            return true;
        }

        String base64URLEncode(uint8_t *buffer) {
            base64 bs;
            String str = bs.encode(buffer, SHA256_LEN);

            str.replace("+", "-");
            str.replace("/", "_");
            str.replace("=", "");
            
            return str;
        }

        void sha256(const char *inBuff, uint8_t *outBuff) {
            SHA256 hash;
            hash.reset();
            hash.update(inBuff, strlen(inBuff));
            hash.finalize(outBuff, SHA256_LEN);
        }

        bool isAuthorized() {
            return accessToken.length() != 0;
        }

        bool isAuthenticated() {
            unsigned long now = millis();
            unsigned long timeElapsed = max(now, tokenIssueMS) - min(now, tokenIssueMS);
            return timeElapsed < expiresIn && this->refreshToken.length() != 0;
        }

        String getSS3AuthURL() {
            SS_LOG_LINE("Code Verifier:     %s", codeVerifier.c_str());
            SS_LOG_LINE("Code Challenge:    %s", codeChallenge.c_str());    
            String ecnodedRedirect = String(SS_OAUTH_REDIRECT_URI);
            ecnodedRedirect.replace(":", "%3A");
            ecnodedRedirect.replace("/", "%2F");
            return String(SS_OAUTH_AUTH_URL) +
                "?client_id=" + SS_OAUTH_CLIENT_ID +
                "&scope=" + SS_OAUTH_SCOPE +
                "&response_type=code" +
                "&redirect_uri=" + ecnodedRedirect +
                "&code_challenge_method=S256" +
                "&code_challenge=" + codeChallenge +
                "&audience=" + SS_OAUTH_AUDIENCE +
                "&auth0Client=" + SS_OAUTH_AUTH0_CLIENT
            ;
        }

        DynamicJsonDocument request(String url, bool post = false, String payload = "", const DynamicJsonDocument &headers = StaticJsonDocument<0>(), int docSize = 3072) {
            LittleFS.begin(); // for SSL certs
            if (https->begin(*client, url)) SS_LOG_LINE("Connected to %s", url.c_str());

            if (headers.size() != 0) {
                for (int x = 0; x < headers.size(); x++) {
                    https->addHeader(headers[x]["name"], headers[x]["value"]);
                    SS_LOG_LINE("Header added... %s: %s", headers[x]["name"].as<const char*>(), headers[x]["value"].as<const char*>());
                }
            }

            SS_LOG_LINE("Payload: %s", payload.c_str());

            int response;
            if (post)
                response = https->POST(payload);
            else
                response = https->GET();

            if (response < 200 || response > 299) {
                SS_LOG_LINE("Error, code: %i.", response);
                SS_LOG_LINE("Response: %s", https->getString().c_str());
                return StaticJsonDocument<0>();
            }

            SS_LOG_LINE("Response: %i", response);
            
            DynamicJsonDocument doc(docSize);
            SS_LOG_LINE("Created doc of %i size", docSize);
            DeserializationError err = deserializeJson(doc, https->getStream());
            SS_LOG_LINE("Desearialized stream.");
            if (err) {
                SS_LOG_LINE("API request deserialization error: %s", err.f_str());
            } else {
                #if SS_DEBUG
                    serializeJsonPretty(doc, Serial);
                    Serial.println("");
                #endif
            }

            client->stop();
            https->end();
            LittleFS.end();
            return doc;
        }

        bool getAuthToken(String code) {
            StaticJsonDocument<256> headers; // optimise size
            headers[0]["name"] = "Host";
            headers[0]["value"] = "auth.simplisafe.com";
            headers[1]["name"] = "Content-Type";
            headers[1]["value"] = "application/json";
            headers[2]["name"] = "Content-Length";
            headers[2]["value"] = "186";
            headers[3]["name"] = "Auth0-Client";
            headers[3]["value"] = SS_OAUTH_AUTH0_CLIENT;

            StaticJsonDocument<256> payloadDoc;
            String payload;
            payloadDoc["grant_type"] = "authorization_code";
            payloadDoc["client_id"] = SS_OAUTH_CLIENT_ID;
            payloadDoc["code_verifier"] = codeVerifier;
            code.replace("\n", "");
            payloadDoc["code"] = code;
            payloadDoc["redirect_uri"] = SS_OAUTH_REDIRECT_URI;
            serializeJson(payloadDoc, payload);
            
            DynamicJsonDocument res = request(SS_OAUTH + String("/token"), true, payload, headers, 3072);

            if (res.size() != 0)
                return storeAuthToken(res);
            else
                return false;
        }

        bool refreshAuthToken() {
            StaticJsonDocument<256> headers; // optimise size
            headers[0]["name"] = "Host";
            headers[0]["value"] = "auth.simplisafe.com";
            headers[1]["name"] = "Content-Type";
            headers[1]["value"] = "application/json";
            headers[2]["name"] = "Content-Length";
            headers[2]["value"] = "186";
            headers[3]["name"] = "Auth0-Client";
            headers[3]["value"] = SS_OAUTH_AUTH0_CLIENT;

            StaticJsonDocument<256> payloadDoc;
            String payload;
            payloadDoc["grant_type"] = "refresh_token";
            payloadDoc["client_id"] = SS_OAUTH_CLIENT_ID;
            payloadDoc["refresh_token"] = refreshToken;
            serializeJson(payloadDoc, payload);

            DynamicJsonDocument res = request(SS_OAUTH + String("/token"), true, payload, headers, 3072);

            if (res.size() != 0)
                return storeAuthToken(res);
            else
                return false;
        }

        bool storeAuthToken(const DynamicJsonDocument &doc) {
            accessToken = doc["access_token"].as<String>();
            refreshToken = doc["refresh_token"].as<String>();
            tokenType = doc["token_type"].as<String>();
            tokenIssueMS = millis();
            expiresIn = doc["expires_in"].as<unsigned long>() * 1000;

            return writeUserData();
        }

        bool writeUserData() {
            // store accessToken, codeVerifier, refreshToken here
            DynamicJsonDocument userData(1536);
            SS_LOG_LINE("Created user data object");

            userData["accessToken"] = accessToken;
            userData["refreshToken"] = refreshToken;
            userData["codeVerifier"] = codeVerifier;

            if (!LittleFS.begin()) {
                SS_LOG_LINE("Error starting LittleFS.");
                return false;
            }

            File file = LittleFS.open(SS_USER_DATA_FILE, "w");
            if (!file) {
                SS_LOG_LINE("Failed to open %s.", SS_USER_DATA_FILE);
                LittleFS.end();
                return false;
            }

            if (serializeJson(userData, file) == 0) {
                SS_LOG_LINE("Failed to write data to %s.", SS_USER_DATA_FILE);
                file.close();
                LittleFS.end();
                return false;
            }

            file.close();
            LittleFS.end();
            return true;
        }

        bool readUserData() {
            if (!LittleFS.begin()) {
                SS_LOG_LINE("Error starting LittleFS.");
                return false;
            }

            File file = LittleFS.open(SS_USER_DATA_FILE, "r");
            if (!file) {
                SS_LOG_LINE("Failed to open %s.", SS_USER_DATA_FILE);
                LittleFS.end();
                return false;
            }

            DynamicJsonDocument userData(1536);
            DeserializationError err = deserializeJson(userData, file);
            if (err) {
                SS_LOG_LINE("Error deserializing %s.", SS_USER_DATA_FILE);
                file.close();
                LittleFS.end();
                return false;
            }

            accessToken = userData["accessToken"].as<String>();
            refreshToken = userData["refreshToken"].as<String>();
            codeVerifier = userData["codeVerifier"].as<String>();

            SS_LOG_LINE("Found user data file...");
            #if SS_DEBUG
                serializeJsonPretty(userData, Serial);
                Serial.println("");
            #endif

            file.close();
            LittleFS.end();
            return true;
        }
};

#endif