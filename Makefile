TARGET?=ESP32
ifeq ("$(TARGET)", "ESP32S3")
PORT?=/dev/ttyACM0
ESP32_DATA=data32
ESP32_DEBUGLEVEL=verbose
ESP32_PSRAM=disabled
ESP32_FLASHSIZE=4MB
ESP32S3_CDCONBOOT=cdc
#ESP32_PARTFILE=partitions_ESP32S3.csv
ESP32_PARTSCHEME=min_spiffs
else
PORT?=/dev/ttyUSB0
ESP32_PSRAM=disabled
ESP32_PARTSCHEME=min_spiffs
ESP32_FLASHSIZE=4MB
endif

ESP32_DEBUGLEVEL=verbose

GITHUB_REPOS= \
reeltwo/Reeltwo \
FastLED/FastLED

ESP32_FILESYSTEM=spiffs
ESP32_FILESYSTEM_PART=spiffs

# FastLED will hang without this
ARDUINO_OPTS+='-prefs="compiler.cpp.extra_flags=-DFASTLED_RMT_MAX_CHANNELS=1"'

ifeq ("$(TARGET)", "ESP32S3")
ARDUINO_OPTS+='-prefs="compiler.cpp.extra_flags=-DWREACTOR32_2023=1"'
endif

include ../Arduino.mk

