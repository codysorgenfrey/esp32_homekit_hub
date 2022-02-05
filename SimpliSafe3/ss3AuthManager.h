#ifndef __SS3AUTHMANAGER_H__
#define __SS3AUTHMANAGER_H__

// TODO: store account in eeprom

#include "ssCommon.h"
#include <ESP8266HTTPclient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Regexp.h>
#include <base64.h>
#include <crypto.h>
#include <SHA256.h>

#define SHA256_LEN 32

class SS3AuthManager {
    private:
        uint8_t randData[32]; // 32 bytes, u_int8_t is 1 byte
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
                https = inHttps;
                client = inClient;
            } else {
                SS_LOG_LINE("No https setup.");
                return false;
            }
            
            https->useHTTP10(true); // for ArduinoJson
            https->setReuse(false); // to reuse connection

            // read in eeprom for accessToken, refreshToken, codeVerifier

            if (codeVerifier.length() == 0) {
                ESP.random(randData, SHA256_LEN);
                codeVerifier = base64URLEncode(randData);
            }
            uint8_t hashOut[SHA256_LEN];
            sha256(codeVerifier.c_str(), hashOut);
            codeChallenge = base64URLEncode(hashOut);

            return true;
        }

        String base64URLEncode(uint8_t *buffer) {
            base64 bs;
            String str = bs.encode(buffer, SHA256_LEN);

            char buff[str.length()];
            str.toCharArray(buff, sizeof(buff));
            MatchState ms(buff);
            ms.GlobalReplace("\+", "-");
            ms.GlobalReplace("\/", "_");
            ms.GlobalReplace("=", "");
            
            return String(buff);
        }

        char *btoh(char *dest, uint8_t *src, int len) {
            char *d = dest;
            while (len--)
                sprintf(d, "%02x", (unsigned char)*src++), d += 2;
            return dest;
        }

        void sha256(const char *inBuff, uint8_t *outBuff) {
            SHA256 hash;
            hash.reset();
            hash.update(inBuff, strlen(inBuff));
            hash.finalize(outBuff, SHA256_LEN);
        }

        bool isAuthorized() {
            return refreshToken.length() != 0;
        }

        bool isAuthenticated() {
            // return millis() < expiry; // fix this to work with millis
        }

        String getSS3AuthURL() {
            char hex[sizeof(randData) * 2];
            btoh(hex, randData, sizeof(randData));
            SS_LOG_LINE("Random String[%u]: %s", sizeof(randData), hex);
            SS_LOG_LINE("Code Verifier:     %s", codeVerifier.c_str());
            SS_LOG_LINE("Code Challenge:    %s", codeChallenge.c_str());
            return String(SS_OAUTH_AUTH_URL) +
                "?client_id=" + SS_OAUTH_CLIENT_ID +
                "&scope=" + SS_OAUTH_SCOPE +
                "&response_type=code" +
                "&redirect_uri=" + SS_OAUTH_REDIRECT_URI +
                "&code_challenge_method=S256" +
                "&code_challenge=" + codeChallenge +
                "&audience=" + SS_OAUTH_AUDIENCE +
                "&auth0Client=" + SS_OAUTH_AUTH0_CLIENT
            ;
        }

        DynamicJsonDocument request(bool post = false, String payload = "", int docSize = 3074) {
            int response;
            if (post)
                response = https->POST(payload);
            else
                response = https->GET();

            if (response < 200 || response > 299) {
                SS_LOG_LINE("Error, response: %i.", response);
                SS_LOG_LINE("%s", https->getString().c_str());
                return StaticJsonDocument<0>();
            }

            SS_LOG_LINE("Response: %i", response);
            
            DynamicJsonDocument doc(docSize); // TODO: optimize size
            DeserializationError err = deserializeJson(doc, https->getStream());
            if (err) {
                SS_LOG_LINE("API request deserialization error: %s", err.f_str());
            } else {
                #if SS_DEBUG
                    serializeJsonPretty(doc, Serial);
                    SS_LOG_LINE("");
                #endif
            }

            client->stop();
            https->end();
            return doc;
        }

        bool getAuthToken(const String code) {
            https->begin(*client, SS_OAUTH + String("/token"));
            https->addHeader("Host", "auth.simplisafe.com");
            https->addHeader("Content-Type", "application/json");
            https->addHeader("Content-Length", "186");
            https->addHeader("Auth0-Client", SS_OAUTH_AUTH0_CLIENT);

            String payload = "{\
                'grant_type':'authorization_code',\
                'client_id':'" + String(SS_OAUTH_CLIENT_ID) + "',\
                'code_verifier':'" + codeVerifier + "',\
                'code':'" + code + "',\
                'redirect_uri':'" + SS_OAUTH_REDIRECT_URI + "'\
            }";
            
            DynamicJsonDocument res = request(true, payload, 3074); // optimise size

            if (res.size() != 0)
                return storeAuthToken(res);
            else
                return false;
        }

        bool refreshAuthToken() {
            https->begin(*client, SS_OAUTH + String("/token"));
            https->addHeader("Host", "auth.simplisafe.com");
            https->addHeader("Content-Type", "application/json");
            https->addHeader("Content-Length", "186");
            https->addHeader("Auth0-Client", SS_OAUTH_AUTH0_CLIENT);

            DynamicJsonDocument doc(256);
            String payload = "{\
                grant_type:refresh_token,\
                client_id:" + String(SS_OAUTH_CLIENT_ID) + ",\
                refresh_token:" + refreshToken + "\
            }";

            DynamicJsonDocument res = request(true, payload, 3074); // optimise size

            if (res.size() != 0)
                return storeAuthToken(res);
            else
                return false;
        }

        bool storeAuthToken(DynamicJsonDocument doc) {
            accessToken = doc["access_token"].as<String>();
            refreshToken = doc["refresh_token"].as<String>();
            tokenType = doc["token_type"].as<String>();
            tokenIssueMS = millis();
            expiresIn = doc["expires_in"].as<unsigned long>();

            // store accessToken, codeVerifier, refreshToken in eeprom here

            return true;
        }
};

#endif