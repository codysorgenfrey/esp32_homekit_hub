# ESP32 HomeKit Hub
An ESP32 bridge that connects some smart shit around the house to Apple Homekit. Requires a butt ton of program space, I used a LOLIN D32 Pro with 4MB app space.

## Dependencies

1. [HomeSpan](https://github.com/HomeSpan/HomeSpan)
2. [Arduino WebSockets](https://github.com/Links2004/arduinoWebSockets)
3. [Arduino JSON](https://github.com/bblanchon/ArduinoJson)
4. [ESP32 SimpliSafe3](https://github.com/codysorgenfrey/esp32_simplisafe3) 
5. [ESP32 MyQ](https://github.com/codysorgenfrey/esp32_myq)

## Required secrets.h
```
#pragma once
#ifndef __SECRETS_H__
#define __SECRETS_H__

#define WIFI_SSID ""
#define WIFI_PASS ""
#define WM_IP ""
#define MYQ_SERIAL ""
#define SHEETS_URL ""
#define WEBSOCKET_USER ""
#define WEBSOCKET_PASS ""

#endif
```