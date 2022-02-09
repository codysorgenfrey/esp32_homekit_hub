#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>

// Board stuff
#define STATUS_LED BUILTIN_LED

// Homekit stuff
#define HK_DEBUG true
#define HK_MANUFACTURER "Lolin"
#define HK_NAME "Smart Home Hub"
#define HK_MODEL "LOLIN D32 PRO"
#define HK_SERIALNUM "2020DP1674"
#define HK_UNIQUE_NAME HK_NAME "-" HK_SERIALNUM
#define HK_SKETCH_VER "0.0.1"

// WeMo Stuff
#define WM_MANUFACTURER "Belkin"
#define WM_NAME "WeMo Smart Switch"
#define WM_MODEL "F7C027"
#define WM_SERIALNUM "221620K01028B9"
#define WM_IP "192.168.86.42"
#define WM_UPDATE_INTERVAL 300000 // 5 minutes

// SimpliSafe
#define SS_DEBUG true
#define SS_MANUFACTURER "SimpliSafe"
#define SS_NAME "SimpliSafe Alarm"
#define SS_MODEL ""
#define SS_SERIALNUM ""
#define SS_UPDATE_INTERVAL 300000 // 5 minutes

// Logging
#if HK_DEBUG
#define HK_LOG(message, ...) printf(">>> [%7d][%.2fkb] HomeKit Hub: " message, millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#define HK_LOG_LINE(message, ...) printf(">>> [%7d][%.2fkb] HomeKit Hub: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
#define HK_LOG(message, ...)
#define HK_LOG_LINE(message, ...)
#endif

#endif