TARGET=ESP32
PORT=/dev/ttyUSB0
#ESP32_FILESYSTEM=littlefs
#ESP32_TYPE=esp32wrover
ESP32_PSRAM=disabled
ESP32_FILESYSTEM=spiffs
ESP32_FILESYSTEM_PART=spiffs
ESP32_PARTSCHEME=min_spiffs
ESP32_FLASHSIZE=4M
GITHUB_REPOS= \
reeltwo/Reeltwo \
FastLED/FastLED

include ../Arduino.mk
