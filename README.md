# ESP8266 HomeKit Hub

## Dependencies

1. ArduinoJson 6.19.1
2. Crypto 0.2.0
3. HomeKit-ESP8266 1.4.0
4. WiFiManager 0.16

## Uploading certs for SSL into SPIFFS

1. Make sure the following are in the PATH:  
ar.exe (C:\Users\cosorgen\AppData\Local\Arduino15\packages\esp8266\tools\xtensa-lx106-elf-gcc\3.0.4-gcc10.3-1757bed\xtensa-lx106-elf\bin)  
openssl.exe (C:\Program Files\Git\usr\bin)  
  
2. run downloadCerts.py

<!-- 3. make sure you have ESP8266FS tool installed (https://github.com/esp8266/arduino-esp8266fs-plugin/releases) -->

3. make sure you have the ESP8266LittleFS tool installed (https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases)  

4. Use tool in Arduino IDE to upload file in "data" folder generated by python script