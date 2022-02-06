#ifndef __SSCOMMON_H__
#define __SSCOMMON_H__

#include <esp_xpgm.h>

#ifndef SS_DEBUG
#define SS_DEBUG true
#endif

// API constants
#define SS3API "https://api.simplisafe.com/v1"
#define SS_OAUTH "https://auth.simplisafe.com/oauth"
#define SS_OAUTH_AUTH0_CLIENT "eyJuYW1lIjoiQXV0aDAuc3dpZnQiLCJlbnYiOnsiaU9TIjoiMTUuMCIsInN3aWZ0IjoiNS54In0sInZlcnNpb24iOiIxLjMzLjAifQ"
#define SS_OAUTH_CLIENT_ID "42aBZ5lYrVW12jfOuu3CQROitwxg9sN5"
#define SS_OAUTH_AUTH_URL "https://auth.simplisafe.com/authorize"
#define SS_OAUTH_REDIRECT_URI "com.simplisafe.mobile://auth.simplisafe.com/ios/com.simplisafe.mobile/callback"
#define SS_OAUTH_SCOPE "offline_access%20email%20openid%20https://api.simplisafe.com/scopes/user:platform"
#define SS_OAUTH_AUDIENCE "https://api.simplisafe.com/"

// Logging
#if SS_DEBUG
#define SS_LOG(message, ...) XPGM_PRINTF(">>> [%7d][%4fkb] SimpliSafe: " message , millis(), (system_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#define SS_LOG_LINE(message, ...) XPGM_PRINTF(">>> [%7d][%4fkb] SimpliSafe: " message "\n", millis(), (system_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
#define SS_LOG(message, ...)
#define SS_LOG_LINE(message, ...)
#endif

#endif