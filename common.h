#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>
#include "secrets.h"
#include <SheetsLogger.h>
#include <stdarg.h>

// Board stuff
#define STATUS_LED BUILTIN_LED

// Homekit stuff
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
#define WM_UPDATE_INTERVAL 60000 // 1 minute

// SimpliSafe
#define SS_MANUFACTURER "SimpliSafe"
#define SS_NAME "SimpliSafe Alarm"
#define SS_MODEL "Soundin"
#define SS_SERIALNUM "16171819"

// Smart Lock
#define SL_MANUFACTURER "SimpliSafe"
#define SL_NAME "SimpliSafe Smart Lock"
#define SL_MODEL "SSLK1BB"
#define SL_SERIALNUM "0104d22a"

// Garage Door
#define GD_MANUFACTURER "Liftmaster"
#define GD_NAME "Liftmaster Garage Door Opener"
#define GD_MODEL "Superlift"
#define GD_SERIALNUM "CG28634B59AD"
#define GD_UPDATE_INTERVAL 60000 // 1 minute

// Temp Sensor
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
#define WA_UPDATE_INTERVAL 1800000 // 30 minutes

// Logging
#define HK_DEBUG_LEVEL_ERROR 0
#define HK_DEBUG_LEVEL_INFO 1

#define HK_DEBUG HK_DEBUG_LEVEL_INFO

void hk_printf(bool cloud, const char *cloudMsg, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    if (cloud) sheetLog(SHEETS_URL, "Homekit Hub", cloudMsg);
}

#if HK_DEBUG >= HK_DEBUG_LEVEL_ERROR
    #define HK_ERROR_LINE(message, ...) hk_printf(true, message, "!!!ERROR!!! [%7d][%.2fkb] HomeKit Hub: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
    #define HK_ERROR_LINE(message, ...)
#endif

#if HK_DEBUG >= HK_DEBUG_LEVEL_INFO
    #define HK_LOG_LINE(message, ...) printf(">>> [%7d][%.2fkb] HomeKit Hub: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
    #define HK_LOG_LINE(message, ...)
#endif

#endif