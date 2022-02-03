#ifndef __SS3AUTHMANAGER_H__
#define __SS3AUTHMANAGER_H__

// TODO: store account in eeprom

#include "ssCommon.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "urlTools.h"
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
        String tokenType = "Bearer";
        String accessToken;

        SS3AuthManager() {
            // read in eeprom for accessToken, refreshToken, codeVerifier

            if (this->codeVerifier.length() == 0) {
                ESP.random(this->randData, SHA256_LEN);
                this->codeVerifier = base64URLEncode(this->randData);
            }
            uint8_t hashOut[SHA256_LEN];
            sha256(this->codeVerifier.c_str(), hashOut);
            this->codeChallenge = base64URLEncode(hashOut);
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

        bool refreshCredentials() {
            WiFiClient client;
            HTTPClient http;
            http.useHTTP10(true);

            http.begin(client, SS_OAUTH + String("/token"));
            http.addHeader("Host", "auth.simplisafe.com");
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Content-Length", "186");
            http.addHeader("Auth0-Client", SS_OAUTH_AUTH0_CLIENT);

            int response = http.POST(String("{ \
                grant_type: \"refresh_token\", \
                client_id: " + String(SS_OAUTH_CLIENT_ID) + ", \
                refresh_token:" + this->refreshToken + " \
            }"));

            if (response < 200 && response < 299) {
                SS_LOG_LINE("Error refreshing credentials.");
                return false;
            }

            storeToken(&http);

            http.end();
            client.stop();
            return true;
        }

        bool isAuthorized() {
            return this->refreshToken.length() != 0;
        }

        bool isAuthenticated() {
            // return millis() < this->expiry; // fix this to work with millis
        }

        String getSS3AuthURL() {
            char hex[sizeof(this->randData) * 2];
            btoh(hex, this->randData, sizeof(this->randData));
            SS_LOG_LINE("Random String[%u]: %s", sizeof(this->randData), hex);
            SS_LOG_LINE("Code Verifier:     %s", this->codeVerifier.c_str());
            SS_LOG_LINE("Code Challenge:    %s", this->codeChallenge.c_str());
            return String(SS_OAUTH_AUTH_URL) +
                "?client_id=" + SS_OAUTH_CLIENT_ID +
                "&scope=" + SS_OAUTH_SCOPE +
                "&response_type=code" +
                "&redirect_uri=" + urlencode(SS_OAUTH_REDIRECT_URI) +
                "&code_challenge_method=S256" +
                "&code_challenge=" + this->codeChallenge +
                "&audience=" + SS_OAUTH_AUDIENCE +
                "&auth0Client=" + SS_OAUTH_AUTH0_CLIENT
            ;
        }

        bool getToken(String code) {
            WiFiClient client;
            HTTPClient http;

            http.useHTTP10(true);
            http.begin(client, SS_OAUTH + String("/token"));
            http.addHeader("Host", "auth.simplisafe.com");
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Content-Length", "186");
            http.addHeader("Auth0-Client", SS_OAUTH_AUTH0_CLIENT);

            DynamicJsonDocument doc(384);
            String payload;
            doc["grant_type"] = "authorization_code";
            doc["client_id"] = SS_OAUTH_CLIENT_ID;
            doc["code_verifier"] = this->codeVerifier;
            doc["code"] = code;
            doc["redirect_uri"] = SS_OAUTH_REDIRECT_URI;
            serializeJson(doc, payload);

            int response = http.POST(payload);
            if (response < 200 && response < 299) {
                SS_LOG_LINE("Error getting token, bad response.");
                return false;
            }

            bool success = storeToken(&http);

            http.end();
            client.stop();
            return success;
        }

        bool storeToken(HTTPClient *http) {
            DynamicJsonDocument doc(2048); // TODO: optimize size
            DeserializationError err = deserializeJson(doc, http->getStream());

            // CRASHING HERE FOR SOME REASON, NEED TO SKIP HTTP HEADERS?
            if (err) {
                SS_LOG_LINE("Error storing token, deserialize json error: %s", err.f_str());
                return false;
            }

            SS_LOG("Store token from response: ");
            #if SS_DEBUG
                serializeJsonPretty(doc, Serial);
            #endif
            SS_LOG_LINE("");

            this->accessToken = doc["access_token"].as<String>();
            this->refreshToken = doc["refresh_token"].as<String>();
            this->tokenType = doc["token_type"].as<String>();
            this->tokenIssueMS = millis();
            this->expiresIn = doc["expires_in"].as<unsigned long>();

            // store accessToken, codeVerifier, refreshToken in eeprom here

            return true;
        }
};

#endif