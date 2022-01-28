#ifndef __COMMON_H__
#define __COMMON_H__

#include "secrets.h"

// Board stuff
#define STATUS_LED BUILTIN_LED
#define PWR_LED 16
#define LED_ON LOW
#define LED_OFF HIGH

// Homekit stuff
#define HK_DEBUG true
#define HK_MANUFACTURER "KeeYees"
#define HK_NAME "Smart Home Hub"
#define HK_MODEL "Node MCU 1.0 ESP-12E"
#define HK_SERIALNUM "12345"
#define HK_UNIQUE_NAME HK_NAME "-" HK_SERIALNUM
#define HK_SKETCH_VER "0.0.1"

// State handling
#define STATUS_ERROR 0
#define STATUS_OK 1
#define STATUS_NO_WIFI 2
#define STATUS_NO_OTA 3
#define STATUS_NO_HOMEKIT 4
#define STATUS_NO_HEAT_PUMP 5
#define STATUS_OTA_PROGRESS 6

#endif