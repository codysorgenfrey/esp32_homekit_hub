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
#define SS_MODEL "Soundin"
#define SS_SERIALNUM "16171819"
#define SS_UPDATE_INTERVAL 300000 // 5 minutes

// Smart Lock
#define SL_MANUFACTURER "SimpliSafe"
#define SL_NAME "SimpliSafe Smart Lock"
#define SL_MODEL "Locker room"
#define SL_SERIALNUM "13141516"
#define SL_UPDATE_INTERVAL 300000 // 5 minutes

// Garage Door
#define GD_DEBUG true
#define GD_MANUFACTURER "Liftmaster"
#define GD_NAME "Liftmaster Garage Door Opener"
#define GD_MODEL "Superlift"
#define GD_SERIALNUM "789101112"
#define GD_UPDATE_INTERVAL 300000 // 5 minutes

// Temp Sensor
#define TS_DEBUG true
#define TS_MANUFACTURER "InkBird"
#define TS_NAME "InkBird Temperature Sensor"
#define TS_MODEL "IBS-TH2"
#define TS_SERIALNUM "123456"
#define TS_UPDATE_INTERVAL 60000 // 1 minute

// Weather API
#define WA_MANUFACTURER "Open Weather Map"
#define WA_NAME "Outside Temperature"
#define WA_MODEL "12345"
#define WA_SERIALNUM "123456"
#define WA_UPDATE_INTERVAL 3600000 // 1 hour

// Logging
#if HK_DEBUG
#define HK_LOG(message, ...) printf(">>> [%7d][%.2fkb] HomeKit Hub: " message, millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#define HK_LOG_LINE(message, ...) printf(">>> [%7d][%.2fkb] HomeKit Hub: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
#define HK_LOG(message, ...)
#define HK_LOG_LINE(message, ...)
#endif

#endif