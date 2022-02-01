#ifndef __SS3AUTHMANAGER_H__
#define __SS3AUTHMANAGER_H__

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "urlTools.h"
#include <Regexp.h>
#include <base64.h>

#define SS_OAUTH "https://auth.simplisafe.com/oauth"
#define SS_OAUTH_AUTH0_CLIENT "eyJuYW1lIjoiQXV0aDAuc3dpZnQiLCJlbnYiOnsiaU9TIjoiMTUuMCIsInN3aWZ0IjoiNS54In0sInZlcnNpb24iOiIxLjMzLjAifQ"
#define SS_OAUTH_CLIENT_ID "42aBZ5lYrVW12jfOuu3CQROitwxg9sN5"
#define SS_OAUTH_AUTH_URL "https://auth.simplisafe.com/authorize"
#define SS_OAUTH_REDIRECT_URI "com.simplisafe.mobile://auth.simplisafe.com/ios/com.simplisafe.mobile/callback"
#define SS_OAUTH_SCOPE "offline_access%20email%20openid%20https://api.simplisafe.com/scopes/user:platform"
#define SS_OAUTH_AUDIENCE "https://api.simplisafe.com/"

class SS3AuthManager {
    private:
        bool logging;
        String refreshToken;
        String codeVerifier;
        String codeChallenge;
        unsigned long expiry;

    public:
        String tokenType;
        String accessToken;

        SS3AuthManager(bool inLogging = true) {
            this->logging = inLogging;

            // read in eprom for accessToken, refreshToken, codeVerifier

            if (this->codeVerifier.length() == 0) {
                u_int8_t buff[32];
                ESP.random(buff, 32);
                String randString = bufferToHexString(buff, 32);
                this->codeVerifier = base64URLEncode(randString); // this may be not right
                if (this->logging) {
                    Serial.print(F("Random 32 byte buffer: "));
                    Serial.println(randString);
                    Serial.print(F("Resulting code verifier: "));
                    Serial.println(this->codeVerifier);
                }
            }
            this->codeChallenge = base64URLEncode(sha256(this->codeVerifier));
            if (this->logging) {
                    Serial.print(F("Resulting code challenge: "));
                    Serial.println(this->codeChallenge);
                }
        }

        String base64URLEncode(String str) {
            base64 bs;
            str = bs.encode(str);

            char buff[str.length()];
            str.toCharArray(buff, str.length());
            MatchState ms(buff);
            ms.GlobalReplace("\+", "-");
            ms.GlobalReplace("\/", "_");
            ms.GlobalReplace("=", "");
            return String(buff);
        }

        String bufferToHexString(u_int8_t buff[], __SIZE_TYPE__ size) {
            String newBuff;
            for (int x = 0; x < size; x++) {
                newBuff += String(buff[x], HEX);
            }
            return newBuff;
        }

        String sha256(String buffer) {
            
            return "";
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

            if (response < 200 && response < 299 && this->logging) {
                Serial.println(F("Error refreshing credentials."));
                return false;
            }

            http.end();
            client.stop();
            return true;
        }

        bool isAuthorized() {
            return this->refreshToken.length() != 0;
        }

        bool isAuthenticated() {
            return millis() < this->expiry; // fix this to work with millis
        }

        String getSS3AuthURL() {
            String URL = String(SS_OAUTH_AUTH_URL) +
                "?client_id=" + SS_OAUTH_CLIENT_ID +
                "&scope=SCOPE" + // this prevents url encoding
                "&response_type=code" +
                "&redirect_uri=" + SS_OAUTH_REDIRECT_URI +
                "&code_challenge_method=S256" +
                "&code_challenge=" + this->codeChallenge +
                "&audience=AUDIENCE" +
                "&auth0Client=" + SS_OAUTH_AUTH0_CLIENT
            ;

            // uri encode url
            URL = urlencode(URL);
            URL.replace("SCOPE", SS_OAUTH_SCOPE);
            URL.replace("AUDIENCE", SS_OAUTH_AUDIENCE);
            
            return URL;
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

            int response = http.POST(String("{ \
                grant_type: \"authorization_code\", \
                client_id: " + String(SS_OAUTH_CLIENT_ID) + ", \
                code_verifier:" + this->codeVerifier + " \
                code:" + code + " \
                redirect_uri:" + SS_OAUTH_REDIRECT_URI + " \
            }"));

            if (response < 200 && response < 299 && this->logging) {
                Serial.println(F("Error getting access token."));
                return false;
            }

            http.end();
            client.stop();
            return true;
        }
};

#endif