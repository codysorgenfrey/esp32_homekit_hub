#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>
#include <SheetsLogger.h>
#include "secrets.h"

// Board stuff
#define STATUS_LED BUILTIN_LED

// Homekit stuff
#define HK_MANUFACTURER "Lolin"
#define HK_NAME "Smart Home Hub"
#define HK_MODEL "LOLIN D32 PRO"
#define HK_SERIALNUM "2020DP1674"
#define HK_UNIQUE_NAME HK_NAME "-" HK_SERIALNUM
#define HK_SKETCH_VER "0.0.1"
#define HUB_COMMAND_RESPONSE "response"
#define HUB_RESPONSE_ERROR 0
#define HUB_RESPONSE_OK 1

// WeMo Stuff
#define WM_MANUFACTURER "Belkin"
#define WM_NAME "WeMo Smart Switch"
#define WM_MODEL "F7C027"
#define WM_SERIALNUM "221620K01028B9"
#define WM_UPDATE_INTERVAL 60000 // 1 minute
#define WM_DEVICE_ID "Wemo-Switch"

// SimpliSafe
#define SS_MANUFACTURER "SimpliSafe"
#define SS_NAME "SimpliSafe Alarm"
#define SS_MODEL "SSBS3"
#define SS_SERIALNUM "003EE648"

// Smart Lock
#define SL_MANUFACTURER "SimpliSafe"
#define SL_NAME "SimpliSafe Smart Lock"
#define SL_MODEL "SSLK1BB"
#define SL_SERIALNUM "0104D22A"

// Garage Door
#define GD_MANUFACTURER "Chamberlain Group"
#define GD_NAME "Liftmaster Garage Door Opener"
#define GD_MODEL "84501"
#define GD_UPDATE_INTERVAL 60000 // 1 minute
#define GD_ACTIVE_UPDATE_INTERVAL 10000 // 10 seconds
#define GD_ACTIVE_UPDATE_DURATION 120000 // 2 minutes
#define GD_OBSTRUCTED_DURATION 30000 // 30 seconds

// Temp Sensor
#define TS_MANUFACTURER "InkBird"
#define TS_NAME "InkBird Temperature Sensor"
#define TS_MODEL "IBS-TH2"
#define TS_SERIALNUM "123456"
#define IB_DEVICE_ID "IBS-TH2"

// Heatpump
#define HP_MANUFACTURER "Mitsubishi"
#define HP_NAME "Heatpump"
#define HP_UPSTAIRS_MODEL "MSZ-GL06NA"
#define HP_DOWNSTAIRS_MODEL "MSZ-GL12NA"
#define HP_UPSTAIRS_SERIALNUM "14E32127"
#define HP_DOWNSTAIRS_SERIALNUM "08E03977"

// Logging
#define HEAP_CHECK_INT 86400000 // 1 day

#define HK_DEBUG_LEVEL_NONE -1
#define HK_DEBUG_LEVEL_ERROR 0
#define HK_DEBUG_LEVEL_INFO 1
#define HK_DEBUG_LEVEL_VERBOSE 2

#define HK_DEBUG HK_DEBUG_LEVEL_VERBOSE

#if HK_DEBUG >= HK_DEBUG_LEVEL_ERROR
    #define HK_ERROR_LINE(message, ...) sl_printf(SHEETS_URL, "Homekit Hub", "ERR [%7lu][%.2fkb] HomeKit Hub: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
    #define HK_ERROR_LINE(message, ...)
#endif

#if HK_DEBUG >= HK_DEBUG_LEVEL_INFO
    #define HK_LOG_LINE(message, ...) printf(">>> [%7lu][%.2fkb] HomeKit Hub: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
    #define HK_LOG_LINE(message, ...)
#endif

#if HK_DEBUG >= HK_DEBUG_LEVEL_VERBOSE
    #define HK_VERB_LINE(message, ...) printf(">>> [%7lu][%.2fkb] HomeKit Hub: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
    #define HK_VERB_LINE(message, ...)
#endif

#endif