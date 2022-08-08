
////////////////////////////////
// CONFIGURATION OPTIONS
////////////////////////////////

#define USE_DEBUG                     // Define to enable debug diagnostic
#define USE_WIFI                      // Define to enable Wifi support
#define USE_DROID_REMOTE              // Define for droid remote support
#define USE_SPIFFS                    // Enable read only filesystem
#define USE_LEDLIB 0                  // Require FastLED instead of Adafruit NeoPixel library
#define USE_SDCARD                    // Enable SD card support

#define USE_LOGICS
#ifdef USE_WIFI
#define USE_MDNS
#define USE_OTA
#define USE_WIFI_WEB
#define USE_WIFI_MARCDUINO
//#define LIVE_STREAM
#endif

////////////////////////////////

#define PREFERENCE_REMOTE_ENABLED       "remote"
#define PREFERENCE_REMOTE_HOSTNAME      "rhost"
#define PREFERENCE_REMOTE_SECRET        "remote"

#define PREFERENCE_FLD                  "fld"
#define PREFERENCE_RLD                  "rld"
#define PREFERENCE_WIFI_ENABLED         "wifi"
#define PREFERENCE_WIFI_SSID            "ssid"
#define PREFERENCE_WIFI_PASS            "pass"
#define PREFERENCE_WIFI_AP              "ap"

#define PREFERENCE_MARCSERIAL1          "mserial1"
#define PREFERENCE_MARCSERIAL2          "mserial2"
#define PREFERENCE_MARCSERIAL_PASS      "mserialpass"
#define PREFERENCE_MARCSERIAL_ENABLED   "mserial"

#define PREFERENCE_MARCWIFI_ENABLED     "mwifi"
#define PREFERENCE_MARCWIFI_SERIAL_PASS "mwifipass"

///////////////////////////////////

#if __has_include("build_version.h")
#include "build_version.h"
#else
#define BUILD_VERSION "custom"
#endif

////////////////////////////////

#include "pin-config.h"

////////////////

// Replace with your network credentials
#ifdef USE_WIFI
#define REMOTE_ENABLED       false
#define WIFI_ENABLED         true
// Set these to your desired credentials.
#define WIFI_AP_NAME         "RSeries"
#define WIFI_AP_PASSPHRASE   "Astromech"
#define WIFI_ACCESS_POINT    true  /* true if access point: false if joining existing wifi */
#endif

#define SMQ_HOSTNAME         "RSeries"
#define SMQ_SECRET           "Astromech"

////////////////////////////////

#ifdef USE_DROID_REMOTE
#include "ReelTwoSMQ32.h"
#else
#include "ReelTwo.h"
#endif
#include "Arduino.h"
#include "core/AnalogMonitor.h"
#include "core/DelayCall.h"
#include "core/Marcduino.h"
#ifdef USE_OTA
#include <ArduinoOTA.h>
#endif

////////////////////////////////

#define MARC_SERIAL1_BAUD_RATE          9600
#define MARC_SERIAL2_BAUD_RATE          9600
#define MARC_SERIAL_PASS                true
#define MARC_SERIAL_ENABLED             true
#define MARC_WIFI_ENABLED               true
#define MARC_WIFI_SERIAL_PASS           true

////////////////////////////////

#if defined(USE_LCD_SCREEN) || defined(USE_DROID_REMOTE)
#define USE_MENUS                     // Define if using menu system
#endif

////////////////////////////////

#include "wifi/WifiAccess.h"

#ifdef USE_MDNS
#include <ESPmDNS.h>
#endif
#ifdef USE_WIFI_WEB
#include "wifi/WifiWebServer.h"
#endif
#ifdef USE_WIFI_MARCDUINO
#include "wifi/WifiMarcduinoReceiver.h"
#endif
#ifdef USE_SPIFFS
#include "SPIFFS.h"
#define USE_FS SPIFFS
#elif defined(USE_FATFS)
#include "FFat.h"
#define USE_FS FFat
#elif defined(USE_LITTLEFS)
#include "LITTLEFS.h"
#define USE_FS LITTLEFS
#endif
#include "FS.h"
#include "SPI.h"
#ifdef USE_SDCARD
#include "SD.h"
#endif

#ifdef LIVE_STREAM
#include "AsyncUDP.h"
#endif
#include "dome/LogicEngine.h"
#include "dome/LogicEngineController.h"
#include "ServoDispatchPCA9685.h"
#include "ServoSequencer.h"

#include <Preferences.h>

Preferences preferences;

/////////////////////////////////////////////////////////////////////////

enum LogicEngineFLDType
{
    kLogicEngineNoFLD = 0,
    kLogicEngineDeathStarFLD,
    kLogicEngineDeathStarFLDInverted,
    kLogicEngineKennyFLD,
    kLogicEngineNabooFLD
};

enum LogicEngineRLDType
{
    kLogicEngineNoRLD = 0,
    kLogicEngineCurvedRLD,
    kLogicEngineCurvedRLDInverted,
    kLogicEngineDeathStarRLD,
    kLogicEngineDeathStarRLDStaggerOdd,
    kLogicEngineDeathStarRLDInverted,
    kLogicEngineDeathStarRLDInvertedStaggerOdd,
    kLogicEngineKennyRLD,
    kLogicEngineNabooRLD
};

#ifdef USE_LOGICS
LogicEngineRenderer* FLD;
LogicEngineRenderer* RLD;
LogicEngineFLDType sFLDType;
LogicEngineRLDType sRLDType;
LogicEngineControllerDefault sController(preferences);

void FLD_init()
{
    switch (sFLDType)
    {
        case kLogicEngineDeathStarFLD:
            FLD = new LogicEngineDeathStarFLD<>(LogicEngineFLDDefault, 1);
            break;
        case kLogicEngineDeathStarFLDInverted:
            FLD = new LogicEngineDeathStarFLDInverted<>(LogicEngineFLDDefault, 1);
            break;
        case kLogicEngineKennyFLD:
            FLD = new LogicEngineKennyFLD<>(LogicEngineFLDDefault, 1);
            break;
        case kLogicEngineNabooFLD:
            FLD = new LogicEngineNabooFLD<>(LogicEngineFLDDefault, 1);
            break;
        case kLogicEngineNoFLD:
        default:
            break;
    }
}

bool RLD_needsClock()
{
    return (sRLDType == kLogicEngineCurvedRLD || sRLDType == kLogicEngineCurvedRLDInverted);
}

void RLD_init()
{
    // LogicEngineCurvedRLD and LogicEngineCurvedRLDInverted will use pins
    // REAR_LOGIC_PIN and REAR_LOGIC_CLOCK_PIN. Change all other RLD types to use
    // REAR_LOGIC_CLOCK_PIN instead of the default REAR_LOGIC_PIN.
    // That way they plug in directly to the board. In that case REAR_LOGIC_PIN
    // will be left unused.
    switch (sRLDType)
    {
        default:
        case kLogicEngineNoRLD:
            break;
        case kLogicEngineCurvedRLD:
            RLD = new LogicEngineCurvedRLD<>(LogicEngineRLDDefault, 2);
            break;
        case kLogicEngineCurvedRLDInverted:
            RLD = new LogicEngineCurvedRLDInverted<>(LogicEngineRLDDefault, 2);
            break;
        case kLogicEngineDeathStarRLD:
            RLD = new LogicEngineDeathStarRLD<REAR_LOGIC_CLOCK_PIN>(LogicEngineRLDDefault, 2);
            break;
        case kLogicEngineDeathStarRLDStaggerOdd:
            RLD = new LogicEngineDeathStarRLDStaggerOdd<REAR_LOGIC_CLOCK_PIN>(LogicEngineRLDDefault, 2);
            break;
        case kLogicEngineDeathStarRLDInverted:
            RLD = new LogicEngineDeathStarRLDInverted<REAR_LOGIC_CLOCK_PIN>(LogicEngineRLDDefault, 2);
            break;
        case kLogicEngineDeathStarRLDInvertedStaggerOdd:
            RLD = new LogicEngineDeathStarRLDInvertedStaggerOdd<REAR_LOGIC_CLOCK_PIN>(LogicEngineRLDDefault, 2);
            break;
        case kLogicEngineKennyRLD:
            RLD = new LogicEngineKennyRLD<REAR_LOGIC_CLOCK_PIN>(LogicEngineRLDDefault, 2);
            break;
        case kLogicEngineNabooRLD:
            RLD = new LogicEngineNabooRLD<REAR_LOGIC_CLOCK_PIN>(LogicEngineRLDDefault, 2);
            break;
    }
}

void FLD_selectType(unsigned type)
{
    switch (type)
    {
        default:
        case kLogicEngineNoFLD:
            sFLDType = kLogicEngineNoFLD;
            break;
        case kLogicEngineDeathStarFLD:
        case kLogicEngineDeathStarFLDInverted:
        case kLogicEngineKennyFLD:
        case kLogicEngineNabooFLD:
            sFLDType = (LogicEngineFLDType)type;
            break;
    }
}

void RLD_selectType(unsigned type)
{
    switch (type)
    {
        default:
        case kLogicEngineNoRLD:
            sRLDType = kLogicEngineNoRLD;
            break;
        case kLogicEngineCurvedRLD:
        case kLogicEngineCurvedRLDInverted:
            sRLDType = (LogicEngineRLDType)type;
            break;
        case kLogicEngineDeathStarRLD:
        case kLogicEngineDeathStarRLDStaggerOdd:
        case kLogicEngineDeathStarRLDInverted:
        case kLogicEngineDeathStarRLDInvertedStaggerOdd:
        case kLogicEngineKennyRLD:
        case kLogicEngineNabooRLD:
            sRLDType = (LogicEngineRLDType)type;
            break;
    }
}

void FLD_selectSequence(byte seq, LogicEngineRenderer::ColorVal colorVal = LogicEngineRenderer::kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
{
    if (FLD != NULL)
        FLD->selectSequence(seq, colorVal, speedScale, numSeconds);
}

void RLD_selectSequence(byte seq, LogicEngineRenderer::ColorVal colorVal = LogicEngineRenderer::kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
{
    if (RLD != NULL)
        RLD->selectSequence(seq, colorVal, speedScale, numSeconds);
}

void FLD_selectScrollTextLeft(const char* text, LogicEngineRenderer::ColorVal colorVal = LogicEngineRenderer::kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
{
    if (FLD != NULL)
        FLD->selectScrollTextLeft(text, colorVal, 0, numSeconds);
}

void RLD_selectScrollTextLeft(const char* text, LogicEngineRenderer::ColorVal colorVal = LogicEngineRenderer::kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
{
    if (RLD != NULL)
        RLD->selectScrollTextLeft(text, colorVal, 0, numSeconds);
}

void FLD_setEffectFontNum(unsigned fontNum)
{
    if (FLD != NULL)
        FLD->setEffectFontNum(fontNum);
}

void RLD_setEffectFontNum(unsigned fontNum)
{
    if (RLD != NULL)
        RLD->setEffectFontNum(fontNum);
}

void FLD_setTextMessage(const char* str)
{
    if (FLD != NULL)
        FLD->setTextMessage(str);
}

void RLD_setTextMessage(const char* str)
{
    if (RLD != NULL)
        RLD->setTextMessage(str);
}

void FLD_setEffectWidthRange(float percent)
{
    if (FLD != NULL)
        FLD->setEffectWidthRange(percent);
}

void RLD_setEffectWidthRange(float percent)
{
    if (RLD != NULL)
        RLD->setEffectWidthRange(percent);
}

void FLD_setPeakValueProvider(PeakValueProvider& provider)
{
    if (FLD != NULL)
        FLD->setPeakValueProvider(provider);
}

void RLD_setPeakValueProvider(PeakValueProvider& provider)
{
    if (RLD != NULL)
        RLD->setPeakValueProvider(provider);
}

void FLD_setLogicEffectSelector(LogicEffectSelector selector)
{
    if (FLD != NULL)
        FLD->setLogicEffectSelector(selector);
}

void RLD_setLogicEffectSelector(LogicEffectSelector selector)
{
    if (RLD != NULL)
        RLD->setLogicEffectSelector(selector);
}

String FLDText;
String RLDText;

#endif

/////////////////////////////////////////////////////////////////////////

#include "MarcduinoLogics.h"

const ServoSettings servoSettings[] PROGMEM = {
};

ServoDispatchPCA9685<SizeOfArray(servoSettings)> servoDispatch(servoSettings);
ServoSequencer servoSequencer(servoDispatch);
AnimationPlayer player(servoSequencer);
MarcduinoSerial<> marcduinoSerial(player);

/////////////////////////////////////////////////////////////////////////

#define NUM_LEDS 28*4
uint32_t lastEvent;
CRGB leds[NUM_LEDS];
#ifdef LIVE_STREAM
AsyncUDP udp;
#else
File fxdata;
#endif

#ifdef USE_WIFI
bool wifiEnabled;
bool wifiActive;
bool remoteEnabled;
bool remoteActive;
TaskHandle_t eventTask;
bool otaInProgress;
#endif

enum
{
    SDBITMAP = 100,
    PLASMA,
    METABALLS,
    FRACTAL,
    FADEANDSCROLL
};

#include "effects/BitmapEffect.h"
#include "effects/FadeAndScrollEffect.h"
#include "effects/FractalEffect.h"
#include "effects/MeatBallsEffect.h"
#include "effects/PlasmaEffect.h"

LogicEffect CustomLogicEffectSelector(unsigned selectSequence)
{
    static const LogicEffect sCustomLogicEffects[] = {
        LogicEffectBitmap,
        LogicEffectPlasma,
        LogicEffectMetaBalls,
        LogicEffectFractal,
        LogicEffectFadeAndScroll
    };
    if (selectSequence >= 100 && selectSequence-100 <= SizeOfArray(sCustomLogicEffects))
    {
        return LogicEffect(sCustomLogicEffects[selectSequence-100]);
    }
    return LogicEffectDefaultSelector(selectSequence);
}

/////////////////////////////////////////////////////////////////////////

static bool sVSPIStarted;

bool ensureVSPIStarted()
{
    if (!sVSPIStarted)
    {
        sVSPIStarted = true;
        SPI.begin();
        SPI.setFrequency(1000000);
    }
    return true;
}

#ifdef USE_SDCARD
static bool sSDCardMounted;
#endif

bool mountReadOnlyFileSystem()
{
#ifdef USE_SPIFFS
    return (SPIFFS.begin(true));
#endif
    return false;
}

bool mountWritableFileSystem()
{
#ifdef USE_FATFS
    return (FFat.begin(true, "/fatfs"));
#endif
    return false;
}

bool getSDCardMounted()
{
    return sSDCardMounted;
}

bool mountSDFileSystem()
{
#ifdef USE_SDCARD
    if (!ensureVSPIStarted())
        return false;
    if (SD.begin(SD_CS_PIN))
    {
        DEBUG_PRINTLN("Card Mount Success");
        sSDCardMounted = true;
        return true;
    }
    DEBUG_PRINTLN("Card Mount Failed");
#endif
    return false;
}

void unmountSDFileSystem()
{
#ifdef USE_SDCARD
    if (sSDCardMounted)
    {
        sSDCardMounted = false;
        SD.end();
    }
    if (sVSPIStarted)
    {
        SPI.end();
    }
#endif
}

void unmountFileSystems()
{
    unmountSDFileSystem();
#ifdef USE_FATFS
    FFat.end();
#endif
#ifdef USE_SPIFFS
    SPIFFS.end();
#endif
}

////////////////////////////////

void reboot()
{
    DEBUG_PRINTLN("Restarting...");
    unmountFileSystems();
    preferences.end();
    delay(1000);
    ESP.restart();
}

/////////////////////////////////////////////////////////////////////////

void scan_i2c()
{
    unsigned nDevices = 0;
    for (byte address = 1; address < 127; address++)
    {
        String name = "<unknown>";
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (address == 0x70)
        {
            // All call address for PCA9685
            name = "PCA9685:all";
        }
        if (address == 0x40)
        {
            // Adafruit PCA9685
            name = "PCA9685";
        }
        if (address == 0x14)
        {
            // IA-Parts magic panel
            name = "IA-Parts Magic Panel";
        }
        if (address == 0x20)
        {
            // IA-Parts periscope
            name = "IA-Parts Periscope";
        }
        if (address == 0x16)
        {
            // PSIPro
            name = "PSIPro";
        }

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.print(" ");
            Serial.println(name);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
}

bool mountFileSystem()
{
#ifdef USE_FATFS
    return (USE_FS.begin(false, "/ffat", 3));
#elif defined(USE_LITTLEFS)
    return (USE_FS.begin(false));
#else
    return (USE_FS.begin(true));
#endif
}

void unmountFileSystem()
{
    USE_FS.end();
}

#ifdef USE_WIFI
WifiAccess wifiAccess;
#endif

#ifdef USE_WIFI_MARCDUINO
WifiMarcduinoReceiver wifiMarcduinoReceiver(wifiAccess);
#endif

////////////////////////////////

#ifdef USE_MENUS

#include "Screens.h"
#include "menus/CommandScreen.h"

#ifdef USE_LCD_SCREEN

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define SCREEN_ADDRESS 0x3C

#include "menus/CommandScreenHandlerSSD1306.h"
CommandScreenHandlerSSD1306 sDisplay(sPinManager);

#else

#include "menus/CommandScreenHandlerSMQ.h"
CommandScreenHandlerSMQ sDisplay;

#endif

#include "menus/utility/ChoiceIntArrayScreen.h"
#include "menus/utility/ChoiceStrArrayScreen.h"
#include "menus/utility/UnsignedValueScreen.h"
#include "menus/utility/MenuScreen.h"

#include "menus/SplashScreen.h"
#include "menus/MainScreen.h"
#include "menus/LogicsScreen.h"
#include "menus/SequenceScreen.h"

#endif

////////////////////////////////

#ifdef USE_WIFI_WEB
#include "WebPages.h"
#endif

////////////////////////////////

void setup()
{
    REELTWO_READY();

    if (!preferences.begin("rseries", false))
    {
        DEBUG_PRINTLN("Failed to init prefs");
    }

    mountSDFileSystem();

    delay(200);

    if (getSDCardMounted())
    {
        File binImage = SD.open("/LENGINE.BIN");
        if (binImage)
        {
            Serial.println("Firmware image found on SD card");
            Serial.print("Reflashing");
            Update.begin(binImage.size());
            uint32_t readSize = 0;
            while (binImage.available())
            {
                uint8_t buf = binImage.read();
                Update.write(&buf, 1);
                readSize++;
                if ((readSize % 102400) == 0)
                    Serial.print(".");
            }
            Serial.println("");
            binImage.close();
            // Delete the image file so we don't constantly reflash the box
            SD.remove("/LENGINE.BIN");
            if (Update.end(true))
            {
                Serial.println("Update Success: "); Serial.println(readSize);
                reboot();
            }
        }
    }
    wifiEnabled = wifiActive = preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED);
    remoteEnabled = remoteActive = preferences.getBool(PREFERENCE_REMOTE_ENABLED, REMOTE_ENABLED);
    printf("wifiEnabled:   %d\n", wifiEnabled);
    printf("remoteEnabled: %d\n", remoteEnabled);

    if (preferences.getBool(PREFERENCE_MARCSERIAL_ENABLED, MARC_SERIAL_ENABLED))
    {
        Serial1.begin(preferences.getInt(PREFERENCE_MARCSERIAL1, MARC_SERIAL1_BAUD_RATE), SERIAL_8N1, SERIAL1_RX_PIN, SERIAL1_TX_PIN);
        if (preferences.getBool(PREFERENCE_MARCSERIAL_PASS, MARC_SERIAL_PASS))
            Serial2.begin(preferences.getInt(PREFERENCE_MARCSERIAL2, MARC_SERIAL2_BAUD_RATE), SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN);

        marcduinoSerial.setStream(&Serial1, (preferences.getBool(PREFERENCE_MARCSERIAL_PASS, MARC_SERIAL_PASS)) ? &Serial2 : nullptr);
    }

    // Initialize FLD
    FLD_selectType(preferences.getUChar(PREFERENCE_FLD, kLogicEngineDeathStarFLD));
    FLD_init();

    // Initialize RLD
    RLD_selectType(preferences.getUChar(PREFERENCE_RLD, kLogicEngineCurvedRLD));
    RLD_init();

    Wire.begin();
    SetupEvent::ready();

    if (remoteEnabled)
    {
    #ifdef USE_SMQ
        WiFi.mode(WIFI_MODE_STA);
        if (SMQ::init(preferences.getString(PREFERENCE_REMOTE_HOSTNAME, SMQ_HOSTNAME),
                        preferences.getString(PREFERENCE_REMOTE_SECRET, SMQ_SECRET)))
        {
            printf("Droid Remote Enabled\n");
            SMQ::setHostDiscoveryCallback([](SMQHost* host) {
                if (host->hasTopic("LCD"))
                {
                    printf("Remote Discovered: %s\n", host->getHostName().c_str());
                }
            });

            SMQ::setHostLostCallback([](SMQHost* host) {
                printf("Lost: %s\n", host->getHostName().c_str());
            });
        }
        else
        {
            printf("Failed to activate Droid Remote\n");
        }
    #endif
    }
    else if (wifiEnabled)
    {
    #ifdef USE_WIFI_WEB
        // In preparation for adding WiFi settings web page
        wifiAccess.setNetworkCredentials(
            preferences.getString(PREFERENCE_WIFI_SSID, WIFI_AP_NAME),
            preferences.getString(PREFERENCE_WIFI_PASS, WIFI_AP_PASSPHRASE),
            preferences.getBool(PREFERENCE_WIFI_AP, WIFI_ACCESS_POINT),
            preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED));
    #ifdef USE_WIFI_MARCDUINO
        wifiMarcduinoReceiver.setEnabled(preferences.getBool(PREFERENCE_MARCWIFI_ENABLED, MARC_WIFI_ENABLED));
        if (wifiMarcduinoReceiver.enabled())
        {
            wifiMarcduinoReceiver.setCommandHandler([](const char* cmd) {
                DEBUG_PRINTLN(cmd);
                Marcduino::processCommand(player, cmd);
                if (preferences.getBool(PREFERENCE_MARCWIFI_SERIAL_PASS, MARC_WIFI_SERIAL_PASS) &&
                    preferences.getBool(PREFERENCE_MARCSERIAL_ENABLED, MARC_SERIAL_ENABLED) &&
                    preferences.getBool(PREFERENCE_MARCSERIAL_PASS, MARC_SERIAL_PASS))
                {
                    Serial2.print(cmd); Serial2.print('\r');
                }
            });
        }
    #endif
        wifiAccess.notifyWifiConnected([](WifiAccess &wifi) {
        #ifdef STATUSLED_PIN
            statusLED.setMode(sCurrentMode = kWifiMode);
        #endif
            Serial.print("Connect to http://"); Serial.println(wifi.getIPAddress());
        #ifdef USE_MDNS
            // No point in setting up mDNS if R2 is the access point
            if (!wifi.isSoftAP())
            {
                String mac = wifi.getMacAddress();
                String hostName = mac.substring(mac.length()-5, mac.length());
                hostName.remove(2, 1);
                hostName = String(WIFI_AP_NAME)+String("-")+hostName;
                if (webServer.enabled())
                {
                    Serial.print("Host name: "); Serial.println(hostName);
                    if (!MDNS.begin(hostName.c_str()))
                    {
                        DEBUG_PRINTLN("Error setting up MDNS responder!");
                    }
                }
            }
        #endif
        });
    #endif
    #ifdef USE_OTA
        ArduinoOTA.onStart([]()
        {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
            {
                type = "sketch";
            }
            else // U_SPIFFS
            {
                type = "filesystem";
            }
            DEBUG_PRINTLN("OTA START");
        })
        .onEnd([]()
        {
            DEBUG_PRINTLN("OTA END");
        })
        .onProgress([](unsigned int progress, unsigned int total)
        {
            // float range = (float)progress / (float)total;
        })
        .onError([](ota_error_t error)
        {
            String desc;
            if (error == OTA_AUTH_ERROR) desc = "Auth Failed";
            else if (error == OTA_BEGIN_ERROR) desc = "Begin Failed";
            else if (error == OTA_CONNECT_ERROR) desc = "Connect Failed";
            else if (error == OTA_RECEIVE_ERROR) desc = "Receive Failed";
            else if (error == OTA_END_ERROR) desc = "End Failed";
            else desc = "Error: "+String(error);
            DEBUG_PRINTLN(desc);
        });
    #endif
    }

    // Setup button and trimpot controller
    sController.configure(FLD, RLD);
    sController.setWifiReset([]() {
        DEBUG_PRINTLN("WiFi Changed");
        preferences.putString(PREFERENCE_WIFI_SSID, WIFI_AP_NAME);
        preferences.putString(PREFERENCE_WIFI_PASS, WIFI_AP_PASSPHRASE);
        preferences.putBool(PREFERENCE_WIFI_AP, WIFI_ACCESS_POINT);
        preferences.putBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED);
        reboot();
    });

    sController.setWifiToggle([]() {
        bool wifiEnabled = !preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED);
        if (wifiEnabled)
        {
            DEBUG_PRINTLN("WiFi Enabled");
        }
        else
        {
            DEBUG_PRINTLN("WiFi Disabled");
        }
        preferences.putBool(PREFERENCE_WIFI_ENABLED, wifiEnabled);
        reboot();
    });

    Serial.print("Total heap:  "); Serial.println(ESP.getHeapSize());
    Serial.print("Free heap:   "); Serial.println(ESP.getFreeHeap());
    Serial.print("Total PSRAM: "); Serial.println(ESP.getPsramSize());
    Serial.print("Free PSRAM:  "); Serial.println(ESP.getFreePsram());

#ifdef USE_WIFI_WEB
    wifiAccess.notifyWifiConnected([](WifiAccess &wifi) {
        Serial.print("Connect to http://"); Serial.println(wifi.getIPAddress());
    #ifdef USE_MDNS
        // No point in setting up mDNS if R2 is the access point
        if (!wifi.isSoftAP())
        {
            String mac = wifi.getMacAddress();
            String hostName = mac.substring(mac.length()-5, mac.length());
            hostName.remove(2, 1);
            hostName = "WReactor32-"+hostName;
            if (webServer.enabled())
            {
                Serial.print("Host name: "); Serial.println(hostName);
                if (!MDNS.begin(hostName.c_str()))
                {
                    DEBUG_PRINTLN("Error setting up MDNS responder!");
                }
            }
        }
    #endif
    });
    // For safety we will stop the motors if the web client is connected
    webServer.setConnect([]() {
        // Callback for each connected web client
        // DEBUG_PRINTLN("Hello");
    });
#endif

#ifdef LIVE_STREAM
    udp.onPacket([](AsyncUDPPacket packet) {
        if (packet.length() == sizeof(leds))
        {
            memcpy(leds, packet.data(), sizeof(leds));
            // Serial.println(".");
        }
        else
        {
            Serial.println("BAD length : "+String(packet.length())+" expecting "+String(sizeof(leds)));
        }
    });
#endif

    RLD_setLogicEffectSelector(CustomLogicEffectSelector);
    FLD_setLogicEffectSelector(CustomLogicEffectSelector);

#ifdef LIVE_STREAM
    if (udp.listen(1234)) {
    }
#endif

#ifdef USE_WIFI
    xTaskCreatePinnedToCore(
          eventLoopTask,
          "Events",
          5000,    // shrink stack size?
          NULL,
          1,
          &eventTask,
          0);
#endif
}

////////////////

MARCDUINO_ACTION(DirectCommand, ~RT, ({
    // Direct ReelTwo command
    CommandEvent::process(Marcduino::getCommand());
}))

////////////////

MARCDUINO_ACTION(WifiByeBye, #WIBYE, ({
    if (wifiEnabled)
    {
        preferences.putBool(PREFERENCE_WIFI_ENABLED, false);
        reboot();
    }
}))

////////////////

MARCDUINO_ACTION(WifiHiHi, #WIHI, ({
    if (!wifiEnabled)
    {
        preferences.putBool(PREFERENCE_WIFI_ENABLED, true);
        reboot();
    }
}))

////////////////

MARCDUINO_ACTION(RemoteByeBye, #WIREMOTEBYE, ({
    if (remoteEnabled)
    {
        preferences.putBool(PREFERENCE_REMOTE_ENABLED, false);
        DEBUG_PRINT("Disabling droid remote. ");
        reboot();
    }
}))

////////////////

MARCDUINO_ACTION(RemoteHiHI, #WIREMOTEHI, ({
    if (!remoteEnabled)
    {
        preferences.putBool(PREFERENCE_REMOTE_ENABLED, true);
        preferences.putBool(PREFERENCE_WIFI_ENABLED, false);
        DEBUG_PRINT("Enabling droid remote. ");
        reboot();
    }
}))

////////////////

////////////////

void mainLoop()
{
    AnimatedEvent::process();
#ifdef USE_MENUS
    sDisplay.process();
#endif
}

////////////////

#ifdef USE_WIFI
void eventLoopTask(void* )
{
    for (;;)
    {
        mainLoop();
        vTaskDelay(1);
    }
}
#endif

////////////////

void loop()
{
#ifdef USE_WIFI
    if (wifiActive)
    {
    #ifdef USE_OTA
        ArduinoOTA.handle();
    #endif
    #ifdef USE_WIFI_WEB
        webServer.handle();
    #endif
    }
    else if (remoteActive)
    {
    #ifdef USE_SMQ
        SMQ::process();
    #endif
    }
#else
    mainLoop();
 #ifdef ESP32
    vTaskDelay(1);
 #endif
#endif
}

////////////////

#ifdef USE_SMQ
SMQMESSAGE(DIAL, {
    long newValue = msg.get_int32("new");
    long oldValue = msg.get_int32("old");
    sDisplay.remoteDialEvent(newValue, oldValue);
})

///////////////////////////////////////////////////////////////////////////////

SMQMESSAGE(BUTTON, {
    uint8_t id = msg.get_uint8("id");
    bool pressed = msg.get_uint8("pressed");
    bool repeat = msg.get_uint8("repeat");
    sDisplay.remoteButtonEvent(id, pressed, repeat);
})

///////////////////////////////////////////////////////////////////////////////

SMQMESSAGE(SELECT, {
    DEBUG_PRINTLN("REMOTE ACTIVE");
    sDisplay.setEnabled(true);
    sDisplay.switchToScreen(kMainScreen);
    sMainScreen.init();
})
#endif
