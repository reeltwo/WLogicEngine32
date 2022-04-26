
////////////////////////////////
// CONFIGURATION OPTIONS
////////////////////////////////

#define USE_DEBUG
#define USE_LEDLIB 0
#define USE_SDCARD

// Audio support is Work in progress. CTFFT requires a newer version of the compiler, using the newer compiler causes problems with FastLED
//#define USE_AUDIO
#define USE_WIFI
#define USE_SPIFFS
#define USE_LOGICS
//#define USE_FATFS
//#define USE_LITTLEFS
#ifdef USE_WIFI
#define USE_MDNS
#define USE_OTA
#define USE_WIFI_WEB
#define USE_WIFI_MARCDUINO
//#define LIVE_STREAM
#endif

/* LED preferences */
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

////////////////////////////////

#include "pin-config.h"

////////////////////////////////

#include "ReelTwo.h"
#include "Arduino.h"
#include "core/AnalogMonitor.h"
#include "core/DelayCall.h"
#include "core/Marcduino.h"
#ifdef USE_OTA
#include <ArduinoOTA.h>
#endif

////////////////

// Replace with your network credentials
#ifdef USE_WIFI
#define WIFI_ENABLED         true
// Set these to your desired credentials.
#define WIFI_AP_NAME         "R2D2"
#define WIFI_AP_PASSPHRASE   "Astromech"
#define WIFI_ACCESS_POINT    true  /* true if access point: false if joining existing wifi */
#endif

#define MARC_SERIAL1_BAUD_RATE          9600
#define MARC_SERIAL2_BAUD_RATE          9600
#define MARC_SERIAL_PASS                true
#define MARC_SERIAL_ENABLED             true
#define MARC_WIFI_ENABLED               true
#define MARC_WIFI_SERIAL_PASS           true

#include "wifi/WifiAccess.h"

#ifdef USE_AUDIO
#include "Audio.h"
#endif
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
#ifdef USE_AUDIO
#include "AudioFrequencyBitmap.h"
#endif

#include <Preferences.h>

Preferences preferences;

/////////////////////////////////////////////////////////////////////////

#ifdef USE_AUDIO
Audio audio;
AudioFrequencyBitmap audioBitmap;
#endif

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

#ifdef USE_AUDIO
class AudioFrequencyBitmapPeakValueProvider : public PeakValueProvider
{
public:
    virtual byte getPeakValue() override
    {
        constexpr float baseline = 0;//0.35f;
        byte peak = (max(audioBitmap.get(0.25f, 0.0f) / 255.0f - baseline, 0.0f) / (1.0f - baseline)) * 255;// * LogicEngineDefaults::MAX_BRIGHTNESS;
        return peak;
    }
};
AudioFrequencyBitmapPeakValueProvider audioPeakValue;
#endif
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

TaskHandle_t audioTask;
bool otaInProgress;

enum
{
    SDBITMAP = 100,
    SPECTRUM,
    REACTIVE,
    PLASMA,
    METABALLS,
    FRACTAL,
    FADEANDSCROLL
};

static bool LogicEffectBitmap(LogicEngineRenderer& r)
{
    return false;
    if (r.hasEffectChanged())
    {
    #ifdef USE_SD
    #ifndef LIVE_STREAM
        fxdata = SD.open("/scene.dat");  // read only
        if (fxdata)
        {
            Serial.println("FILE OPENED");
        }
        else
        {
            Serial.println("FILE FAILED TO OPEN");
        }
    #endif
    #endif
        r.setEffectDelay(25);
        r.calculateAllColors();
    }
    if (r.getEffectFlip())
    {
    #ifndef LIVE_STREAM
        if (fxdata.available())
            fxdata.readBytes((char*)leds, NUM_LEDS*3);
        else
            memset(leds, '\0', sizeof(leds));
    #endif
    }
    r.updateDisplay();
    unsigned h = r.height();
    unsigned w = r.width();
    for (unsigned y = 0; y < h; y++)
    {
        for (unsigned x = 0; x < w; x++)
        {
        #ifndef LIVE_STREAM
            CRGB src = leds[x+y*w];
            CRGB dst;
            dst.r = (uint8_t)((float)src.r * 0.5f);
            dst.g = (uint8_t)((float)src.g * 0.5f);
            dst.b = (uint8_t)((float)src.b * 0.5f);
            if (dst.r != 0 || dst.g != 0 || dst.b != 0)
                r.setPixelRGB(x, y, dst);
        #else
            r.setPixelRGB(x, y, leds[x+y*w]);
        #endif
        }
    }
        // static int scount; scount++;
        // Serial.print(leds[0].r, HEX); Serial.print(" ");
        // Serial.print(leds[0].g, HEX); Serial.print(" ");
        // Serial.print(leds[0].b, HEX); Serial.println();
    // }
    return true;
}

// float map(float x, float in_min, float in_max, float out_min, float out_max)
// {
//     return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
// }

static bool LogicEffectSpectrum(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setEffectDelay(25);
        r.calculateAllColors();
    }
#ifdef USE_AUDIO
    unsigned h = r.height();
    unsigned w = r.width();
    unsigned f = 8;
    for (unsigned x = 0; x < w; x++, f += 4)
    {
        int fval = 0;
        constexpr float baseline = 0.35f;
        fval = max(int(audioBitmap.get(f+0, 0)), fval);
        fval = max(int(audioBitmap.get(f+1, 0)), fval);
        fval = max(int(audioBitmap.get(f+2, 0)), fval);
        fval = max(int(audioBitmap.get(f+3, 0)), fval);
        float freq = max((fval / 255.0f) - baseline, 0.0f) / (1.0f - baseline);

        CRGB pixel;
        pixel.g = 0;
        pixel.b = 0;

        constexpr int maxval = LogicEngineDefaults::MAX_BRIGHTNESS;
        int c = freq * 4*maxval;

        int color1 = min(max(c - 3*maxval, 0), maxval);
        int color2 = min(max(c - 2*maxval, 0), maxval);
        int color3 = min(max(c - 1*maxval, 0), maxval);
        int color4 = min(max(c, 0), maxval);

        if (h == 4)
        {
            pixel.r = color1;
            pixel.g = 0;
            pixel.b = 0;
            r.setPixelRGB(x, 0, pixel);
            pixel.r = color2;
            pixel.g = color2;
            pixel.b = 0;
            r.setPixelRGB(x, 1, pixel);
            pixel.r = 0;
            pixel.g = color3;
            pixel.b = color3;
            r.setPixelRGB(x, 2, pixel);
            pixel.r = 0;
            pixel.g = color4;
            pixel.b = 0;
            r.setPixelRGB(x, 3, pixel);
        }
        else
        {
            pixel.r = color1;
            pixel.g = 0;
            pixel.b = 0;
            r.setPixelRGB(x, 0, pixel);
            pixel.r = color1;
            pixel.g = 0;
            pixel.b = 0;
            r.setPixelRGB(x, 1, pixel);
            pixel.r = color1;
            pixel.g = 0;
            pixel.b = 0;
            r.setPixelRGB(x, 2, pixel);
            pixel.r = color2;
            pixel.g = color2;
            pixel.b = 0;
            r.setPixelRGB(x, 3, pixel);
            pixel.r = color2;
            pixel.g = color2;
            pixel.b = 0;
            r.setPixelRGB(x, 4, pixel);
            pixel.r = 0;
            pixel.g = color3;
            pixel.b = color3;
            r.setPixelRGB(x, 5, pixel);
            pixel.r = 0;
            pixel.g = color3;
            pixel.b = color3;
            r.setPixelRGB(x, 6, pixel);
            pixel.r = 0;
            pixel.g = color3;
            pixel.b = color3;
            r.setPixelRGB(x, 7, pixel);
            pixel.r = 0;
            pixel.g = color4;
            pixel.b = 0;
            r.setPixelRGB(x, 8, pixel);
            pixel.r = 0;
            pixel.g = color4;
            pixel.b = 0;
            r.setPixelRGB(x, 9, pixel);
        }
    }
#endif
    return true;
}

static double randomDouble()
{
    return (random(10000)/10000.0);
}

#ifdef USE_AUDIO

static double interpolate(double d1, double d2, double fraction)
{
    return d1 + (d2 - d1) * fraction;
}

static CRGB interpolate(CRGB color1, CRGB color2, double fraction)
{
    CRGB color;
    color.r = int(interpolate(color1.r, color2.r, fraction));
    color.g = int(interpolate(color1.g, color2.g, fraction));
    color.b = int(interpolate(color1.b, color2.b, fraction));
    return color;
}

static CRGB getBlendedColor(float percentage)
{
    CRGB black = { 0, 0, 0 };
    CRGB yellow = { 0xFF, 0xFF, 0 };
    CRGB red = { 0xFF, 0xFF, 0 };
    if (percentage < 0.5f)
        return interpolate(black, yellow, percentage / 0.5f);
    return interpolate(yellow, red, (percentage - 0.5f) / 0.5f);
}
#endif

static bool LogicEffectReactive(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setEffectDelay(50);
        memset(leds, '\0', sizeof(leds));
    }
#ifdef USE_AUDIO
    unsigned h = r.height();
    unsigned w = r.width();
    unsigned f = 8;
    if (r.getEffectFlip() && audioBitmap.isUpdated())
    {
        memcpy(leds, &leds[w], w*sizeof(leds[0])*(h-1));
    }
    for (unsigned x = 0; x < w; x++, f+= 4)
    {
        int fval = 0;
        constexpr float baseline = 0.50f;
        fval = max(int(audioBitmap.get(f+0, 0)), fval);
        fval = max(int(audioBitmap.get(f+1, 0)), fval);
        fval = max(int(audioBitmap.get(f+2, 0)), fval);
        fval = max(int(audioBitmap.get(f+3, 0)), fval);
        float freq = max((fval / 255.0f) - baseline, 0.0f) / (1.0f - baseline);

        // CRGB pixel ;
        // pixel.r = min(127, int(127 * freq));
        // pixel.g = min(127, int(2 * 127 * (1-freq)));
        // pixel.b = 0;
        leds[x+3*w] = getBlendedColor(freq);
        // r.setPixelRGB(x, 4, pixel);
    }
    for (unsigned y = 0; y < h; y++)
    {
        for (unsigned x = 0; x < w; x++)
        {
            r.setPixelRGB(x, y, leds[x+y*w]);
        }
    }
#endif
    return true;
}

static bool LogicEffectPlasma(LogicEngineRenderer& r)
{
    class PlasmaObject : public LogicEffectObject
    {
    public:
        int plasma_step_width = 1;
        int plasma_cell_size_x = 3;
        int plasma_cell_size_y = 3;
        uint8_t plasma_lut[1537][3];
        float plasma_counter = 0.0F;

        PlasmaObject()
        {
            int i;
            for (i = 0; i < 256; i++)
            {
                plasma_lut[i + 0][0] = 255;
                plasma_lut[i + 0][1] = i;
                plasma_lut[i + 0][2] = 0;
            }
            for (i = 0; i < 256; i++)
            {
                plasma_lut[i + 256][0] = 255 - i;
                plasma_lut[i + 256][1] = 255;
                plasma_lut[i + 256][2] = 0;
            }
            for (i = 0; i < 256; i++)
            {
                plasma_lut[i + 512][0] = 0;
                plasma_lut[i + 512][1] = 255;
                plasma_lut[i + 512][2] = i;
            }
            for (i = 0; i < 256; i++)
            {
                plasma_lut[i + 768][0] = 0;
                plasma_lut[i + 768][1] = 255 - i;
                plasma_lut[i + 768][2] = 255;
            }
            for (i = 0; i < 256; i++)
            {
                plasma_lut[i + 1024][0] = i;
                plasma_lut[i + 1024][1] = 0;
                plasma_lut[i + 1024][2] = 255;
            }
            for (i = 0; i < 256; i++)
            {
                plasma_lut[i + 1280][0] = 255;
                plasma_lut[i + 1280][1] = 0;
                plasma_lut[i + 1280][2] = 255 - i;
            }
        }
    };

    if (r.hasEffectChanged())
    {
        r.setEffectObject(new PlasmaObject());
        r.clear();
    }
    PlasmaObject* obj = (PlasmaObject*)r.getEffectObject();
    unsigned h = r.height();
    unsigned w = r.width();

    obj->plasma_counter += obj->plasma_step_width / 10.0;
    double calc1 = sin((obj->plasma_counter * 0.006));
    double calc2 = sin((obj->plasma_counter * -0.06));
    double xc = 25.0;
    for (int x = 0; x < w; x++)
    {
        xc += (obj->plasma_cell_size_x / 10.0);
        double yc = 25.0;
        double s1 = 768.0 + 768.0 * sin(xc) * calc1;
        for (int y = 0; y < h; y++)
        {
            yc += (obj->plasma_cell_size_y / 10.0);
            double s2 = 768.0 + 768.0 * sin(yc) * calc2;
            double s3 = 768.0 + 768.0 * sin((xc + yc + (obj->plasma_counter / 10.0)) / 2.0);
            int pixel = (int)((s1 + s2 + s3) / 3.0);
            r.setPixelRGB(
                x, y,
                obj->plasma_lut[pixel][0],
                obj->plasma_lut[pixel][1],
                obj->plasma_lut[pixel][2]);
        }
    }
    return true;
}

static bool LogicEffectMetaBalls(LogicEngineRenderer& r)
{
    class MetaBallsObject : public LogicEffectObject
    {
    public:
        int mb_r_start = 255;
        int mb_g_start = 0;
        int mb_b_start = 255;
        int mb_dia = 1+random(24);
        int mb_number = 1+random(4);
        int mb_speed = 1+random(4);
        bool mb_random_color = true;
        int mb_random_time = 25;
        int* mb_vx;
        int* mb_vy;
        int* mb_px;
        int* mb_py;
        int* mb_dx;
        int* mb_dy;
        int mb_counter = 0;

        MetaBallsObject(LogicEngineRenderer& r)
        {
            unsigned h = r.height();
            unsigned w = r.width();
            mb_px = new int[mb_number];
            mb_py = new int[mb_number];
            mb_dx = new int[mb_number];
            mb_dy = new int[mb_number];
            mb_vx = new int[mb_number * w];
            mb_vy = new int[mb_number * h];
            for (int j = 0; j < mb_number; j++)
            {
                mb_px[j] = (int)(randomDouble() * w);
                mb_py[j] = (int)(randomDouble() * h);
                mb_dx[j] = (randomDouble() < 0.5) ? -1 : 1;
                mb_dy[j] = (randomDouble() < 0.5) ? -1 : 1;
            }
            Serial.println(randomDouble());
            Serial.println(randomDouble());
            Serial.println(randomDouble());
            Serial.println(randomDouble());
            Serial.println(randomDouble());
        }

        virtual ~MetaBallsObject() override
        {
            delete[] mb_px;
            delete[] mb_py;
            delete[] mb_dx;
            delete[] mb_dy;
            delete[] mb_vx;
            delete[] mb_vy;
        }
    };

    if (r.hasEffectChanged())
    {
        r.setEffectObject(new MetaBallsObject(r));
    }
    MetaBallsObject* obj = (MetaBallsObject*)r.getEffectObject();
    unsigned h = r.height();
    unsigned w = r.width();

    int mb_number = obj->mb_number;
    int* mb_vx = obj->mb_vx;
    int* mb_vy = obj->mb_vy;
    int* mb_px = obj->mb_px;
    int* mb_py = obj->mb_py;
    int* mb_dx = obj->mb_dx;
    int* mb_dy = obj->mb_dy;

    if (obj->mb_random_color)
    {
        if (obj->mb_counter > obj->mb_random_time)
        {
            obj->mb_r_start = random(127);
            obj->mb_g_start = random(127);
            obj->mb_b_start = random(127);
            obj->mb_counter = 0;
        }
        else
        {
            obj->mb_counter++;
        }
    }
    for (int i = 0; i < mb_number; i++)
    {
        mb_px[i] = mb_px[i] + obj->mb_speed * mb_dx[i];
        mb_py[i] = mb_py[i] + obj->mb_speed * mb_dy[i];
        if (mb_px[i] < 0)
            mb_dx[i] = 1;
        else if (mb_px[i] > w)
            mb_dx[i] = -1;

        if (mb_py[i] < 0)
            mb_dy[i] = 1;
        else if (mb_py[i] > h)
            mb_dy[i] = -1;
        for (int x = 0; x < w; x++)
            mb_vx[i*mb_number+x] = (mb_px[i] - x) * (mb_px[i] - x);
        for (int y = 0; y < h; y++)
            mb_vy[i*mb_number+y] = (mb_py[i] - y) * (mb_py[i] - y);
    }
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            int R = 0;
            int G = 0;
            int B = 0;
            for (int i = 0; i < mb_number; i++)
            {
                double distance = (mb_vx[i*mb_number+x] + mb_vy[i*mb_number+y] + 1);
                R += (int)(obj->mb_dia / distance * obj->mb_r_start);
                G += (int)(obj->mb_dia / distance * obj->mb_g_start);
                B += (int)(obj->mb_dia / distance * obj->mb_b_start);
            }
            if (R > 255)
                R = 255;
            if (G > 255)
                G = 255;
            if (B > 255)
                B = 255;
            r.setPixelRGB(x, y, R*0.5, G*0.5, B*0.5);
        }
    }
    return true;
}

static bool LogicEffectFractal(LogicEngineRenderer& r)
{
    class FractalObject : public LogicEffectObject
    {
    public:
        int* color;
        int* level;
        int grow_size = 1+random(3);
        bool distortion = true;
        int dist_strength = 10+random(30);

        FractalObject(LogicEngineRenderer& r)
        {
            unsigned h = r.height();
            unsigned w = r.width();
            color = new int[w*h];
            level = new int[w*h];
            memset(color, '\0', sizeof(color[0]) * w * h);
            memset(level, '\0', sizeof(level[0]) * w * h);
            for (int x = 0; x < w; x++)
            {
                color[x + (h / 3 * 1)*w] = 1;
                color[x + (h / 3 * 2)*w] = 3;
            }
            for (int y = 0; y < h; y++)
            {
                color[w / 3 * 1 + y*w] = 2;
                color[w / 3 * 2 + y*w] = 3;
            }
        }

        virtual ~FractalObject() override
        {
            delete[] color;
            delete[] level;
        }
    };

    if (r.hasEffectChanged())
    {
        r.setEffectObject(new FractalObject(r));
    }
    FractalObject* obj = (FractalObject*)r.getEffectObject();
    unsigned h = r.height();
    unsigned w = r.width();

    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            int pos_x = 0;
            int pos_y = 0;
            while (pos_x == 0 && pos_y == 0)
            {
                pos_x = random(3);
                pos_y = random(3);
            }
            pos_x = x - 1 + pos_x;
            pos_y = y - 1 + pos_y;
            if (pos_x < 0)
                pos_x = w - 1;
            if (pos_y < 0)
                pos_y = h - 1;
            if (pos_x > w - 1)
                pos_x = 0;
            if (pos_y > h - 1)
                pos_y = 0;
            if (obj->color[x + y*w] == 0 && obj->color[pos_x + pos_y*w] != 0
                && obj->level[pos_x + pos_y*w] < obj->grow_size)
            {
                obj->color[x + y*w] = obj->color[pos_x + pos_y*w];
                obj->level[x + y*w] = obj->level[pos_x + pos_y*w] + 1;
            }
            else
            {
                switch (obj->color[x + y*w])
                {
                    case 1:
                        if (obj->color[pos_x + pos_y*w] == 3)
                        {
                            obj->level[x + y*w] = obj->level[x + y*w] - 1;
                            obj->level[pos_x + pos_y*w] = obj->level[pos_x + pos_y*w] + 1;
                        }
                        if (obj->color[pos_x + pos_y*w] == 2)
                        {
                            obj->level[x + y*w] = obj->level[x + y*w] + 1;
                            obj->level[pos_x + pos_y*w] = obj->level[pos_x + pos_y*w] - 1;
                        }
                        break;
                    case 2:
                        if (obj->color[pos_x + pos_y*w] == 1)
                        {
                            obj->level[x + y*w] = obj->level[x + y*w] - 1;
                            obj->level[pos_x + pos_y*w] = obj->level[pos_x + pos_y*w] + 1;
                        }
                        if (obj->color[pos_x + pos_y*w] == 3)
                        {
                            obj->level[x + y*w] = obj->level[x + y*w] + 1;
                            obj->level[pos_x + pos_y*w] = obj->level[pos_x + pos_y*w] - 1;
                        }
                        break;
                    case 3:
                        if (obj->color[pos_x + pos_y*w] == 2)
                        {
                            obj->level[x + y*w] = obj->level[x + y*w] - 1;
                            obj->level[pos_x + pos_y*w] = obj->level[pos_x + pos_y*w] + 1;
                        }
                        if (obj->color[pos_x + pos_y*w] == 1)
                        {
                            obj->level[x + y*w] = obj->level[x + y*w] + 1;
                            obj->level[pos_x + pos_y*w] = obj->level[pos_x + pos_y*w] - 1;
                        }
                        break;
                }
                if (obj->level[x + y*w] < 0)
                    obj->level[x + y*w] = 0;
                if (obj->level[x + y*w] > obj->grow_size)
                {
                    obj->color[x + y*w] = obj->color[pos_x + pos_y*w];
                    obj->level[x + y*w] = 0;
                }
            }
        }
    }
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            if (obj->distortion)
            {
                int r = random(10000);
                if (r < obj->dist_strength)
                    obj->color[x + y*w] = random(3) + 1;
            }
            CRGB temp_color;
            temp_color.r = 0;
            temp_color.g = 0;
            temp_color.b = 0;
            switch (obj->color[x + y*w])
            {
                case 1:
                    temp_color.r = 255;
                    break;
                case 2:
                    temp_color.g = 255;
                    break;
                case 3:
                    temp_color.b = 255;
                    break;
            }
            r.setPixelRGB(x, y, temp_color.r, temp_color.g, temp_color.b);
        }
    }
    return true;
}

static inline int iabs(int a)
{
    return (a < 0) ? -a : a;
}

static bool LogicEffectFadeAndScroll(LogicEngineRenderer& r)
{
    enum Palette
    {
        kPaletteRGB,
        kPaletteRed,
        kPaletteGreen,
        kPaletteBlue,
        kPaletteWhite,
        kPaletteHalf,
        kPaletteLast = kPaletteHalf,
        kPaletteRandom
    };
    enum Direction
    {
        kForward,
        kBackward,
        kLastDirection = kBackward,
        kRandomDirection
    };
    enum Type
    {
        kFlat,
        kVertical,
        kHorizontal,
        kDiagonalLeft,
        kDiagonalRight,
        kHorizontalSymmetric,
        kVerticalSymmetric,
        kHyperbola,
        kDiamond,
        kCircle,
        kPlasma,
        kTypeLast = kPlasma,
        kRandomType
    };
    class FadeObject : public LogicEffectObject
    {
    public:
        int fs_speed = 1+random(10);
        int fs_zoom = 5+random(40);
        int fs_index = 0;
        Type fs_scroll_type = kRandomType;
        Direction fs_dir = kRandomDirection;
        Palette fs_palette = random(1) ? kPaletteRGB : random(1) ? kPaletteHalf : Palette(random(kPaletteLast));
        int* fs_height = NULL;
        CRGB* fs_lut = NULL;
        int fs_lut_len = 0;

        FadeObject(LogicEngineRenderer& r)
        {
            unsigned h = r.height();
            unsigned w = r.width();

            if (fs_palette == kPaletteRandom)
                fs_palette = Palette(random(int(kPaletteLast)));
            if (fs_dir == kRandomDirection)
                fs_dir = Direction(random(int(kLastDirection)));
            if (fs_scroll_type == kRandomType)
                fs_scroll_type = Type(random(int(kTypeLast)));
            switch (fs_palette)
            {
                case kPaletteRGB:
                    fs_lut = new CRGB[fs_lut_len = 1536];
                    for (int i = 0; i < 256; i++)
                    {
                        fs_lut[i].setRGB(255, i, 0);
                        fs_lut[i + 256].setRGB(255 - i, 255, 0);
                        fs_lut[i + 512].setRGB(0, 255, i);
                        fs_lut[i + 768].setRGB(0, 255 - i, 255);
                        fs_lut[i + 1024].setRGB(i, 0, 255);
                        fs_lut[i + 1280].setRGB(255, 0, 255 - i);
                    }
                    break;
                case kPaletteRed:
                    fs_lut = new CRGB[fs_lut_len = 512];
                    for (int i = 0; i < 256; i++)
                    {
                        fs_lut[i].setRGB(i, 0, 0);
                        fs_lut[i + 256].setRGB(255 - i, 0, 0);
                    }
                    break;
                case kPaletteGreen:
                    fs_lut = new CRGB[fs_lut_len = 512];
                    for (int i = 0; i < 256; i++)
                    {
                        fs_lut[i + 0].setRGB(0, i, 0);
                        fs_lut[i + 256].setRGB(0, 255 - i, 0);
                    }
                    break;
                case kPaletteBlue:
                    fs_lut = new CRGB[fs_lut_len = 512];
                    for (int i = 0; i < 256; i++)
                    {
                        fs_lut[i + 0].setRGB(0, 0, i);
                        fs_lut[i + 256].setRGB(0, 0, 255 - i);
                    }
                    break;
                case kPaletteWhite:
                    fs_lut = new CRGB[fs_lut_len = 512];
                    for (int i = 0; i < 256; i++)
                    {
                        fs_lut[i + 0].setRGB(i, i, i);
                        fs_lut[i + 256].setRGB(255 - i, 255 - i, 255 - i);
                    }
                    break;
                case kPaletteHalf:
                    fs_lut = new CRGB[fs_lut_len = 768];
                    for (int i = 0; i < 128; i++)
                    {
                        fs_lut[i + 0].setRGB(254 - 2 * i, 0, 127 - i);
                        fs_lut[i + 128].setRGB(i, 2 * i, 0);
                        fs_lut[i + 256].setRGB(127 - i, 254 - 2 * i, 0);
                        fs_lut[i + 384].setRGB(0, i, 2 * i);
                        fs_lut[i + 512].setRGB(0, 127 - i, 254 - 2 * i);
                        fs_lut[i + 640].setRGB(2 * i, 0, i);
                    }
                    break;
                case kPaletteRandom:
                    /* ignore */
                    break;
            }
            fs_height = new int[w * h];
            switch (fs_scroll_type)
            {
                case kFlat:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = 0;
                        }
                    }
                    break;
                case kVertical:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = y * fs_zoom % fs_lut_len;
                        }
                    }
                    break;
                case kHorizontal:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = x * fs_zoom % fs_lut_len;
                        }
                    }
                    break;
                case kDiagonalLeft:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = (x + y) * fs_zoom % fs_lut_len;
                        }
                    }
                    break;
                case kDiagonalRight:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = (x + h - y) * fs_zoom % fs_lut_len;
                        }
                    }
                    break;
                case kHorizontalSymmetric:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = iabs(x - w / 2) * fs_zoom % fs_lut_len;
                        }
                    }
                    break;
                case kVerticalSymmetric:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = iabs(y - h / 2) * fs_zoom % fs_lut_len;
                        }
                    }
                    break;
                case kHyperbola:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = iabs(y - h / 2) * iabs(x - w / 2) * fs_zoom / 10 % fs_lut_len;
                        }
                    }
                    break;
                case kDiamond:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = (iabs(y - h / 2) + iabs(x - w / 2)) * fs_zoom % fs_lut_len;
                        }
                    }
                    break;
                case kCircle:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = ((y - h / 2) * (y - h / 2) + (x - w / 2) * (x - w / 2)) * fs_zoom / 10 % fs_lut_len;
                        }
                    }
                    break;
                case kPlasma:
                    for (int x = 0; x < w; x++)
                    {
                        for (int y = 0; y < h; y++)
                        {
                            fs_index = y * w + x;
                            fs_height[fs_index] = (int)(((fs_lut_len / 2.0F) + fs_lut_len / 2.0 * sin((x / fs_zoom)) + (fs_lut_len / 2.0F) + fs_lut_len / 2.0 * sin((y / fs_zoom))) / 2.0);
                        }
                    }
                    break;
                case kRandomType:
                    /* ignore */
                    break;
            }
        }

        virtual ~FadeObject() override
        {
            if (fs_lut != NULL)
                delete[] fs_lut;
            if (fs_height != NULL)
                delete[] fs_height;
        }
    };

    if (r.hasEffectChanged())
    {
        r.setEffectObject(new FadeObject(r));
    }
    FadeObject* obj = (FadeObject*)r.getEffectObject();
    unsigned h = r.height();
    unsigned w = r.width();
    if (obj->fs_height == NULL || obj->fs_lut == NULL)
        return false;

    switch (obj->fs_dir)
    {
        case kForward:
            for (int x = 0; x < w; x++)
            {
                for (int y = 0; y < h; y++)
                {
                    obj->fs_index = y * w + x;
                    obj->fs_height[obj->fs_index] = obj->fs_height[obj->fs_index] + obj->fs_speed;
                    if (obj->fs_height[obj->fs_index] > obj->fs_lut_len - 1)
                        obj->fs_height[obj->fs_index] = 0; 
                    CRGB color = obj->fs_lut[obj->fs_height[obj->fs_index]];
                    r.setPixelRGB(x, y, color.r, color.g, color.b);
                }
            }
            break;
        case kBackward:
            for (int x = 0; x < w; x++)
            {
                for (int y = 0; y < h; y++)
                {
                    obj->fs_index = y * w + x;
                    obj->fs_height[obj->fs_index] = obj->fs_height[obj->fs_index] - obj->fs_speed;
                    if (obj->fs_height[obj->fs_index] < 0)
                        obj->fs_height[obj->fs_index] = obj->fs_lut_len - 1; 
                    CRGB color = obj->fs_lut[obj->fs_height[obj->fs_index]];
                    r.setPixelRGB(x, y, color.r, color.g, color.b);
                }
            }
            break;
        case kRandomDirection:
            /* ignore */
            break;
    }
    return true;
}

LogicEffect CustomLogicEffectSelector(unsigned selectSequence)
{
    static const LogicEffect sCustomLogicEffects[] = {
        LogicEffectBitmap,
        LogicEffectSpectrum,
        LogicEffectReactive,
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

void audioLoopTask(void* arg)
{
    for (;;)
    {
    #ifdef USE_AUDIO
        audio.loop();
    #endif
        if (otaInProgress)
            AnimatedEvent::process();
        vTaskDelay(1);
    }
    // Serial.println("Audio running on core: "+String(xPortGetCoreID()));
    // for (;;)
    //     audio.loop();
}

void playRandom()
{
#if defined(USE_AUDIO) && defined(USE_SD)
    static const char* sounds[] = {
        "/sounds/WOWIE.mp3",
        "/sounds/SWDiscoShort.mp3",
        "/sounds/UptownFunk.mp3",
        "/sounds/LEIA.mp3",
        "/sounds/Gangnam.mp3",
        "/sounds/firefly.mp3"
    };
    audio.connecttoFS(SD, sounds[random(SizeOfArray(sounds))]);
#endif
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

char bintohex(unsigned val)
{
    return (val <= 9) ? '0'+val : (val <= 0xF) ? 'a'+(val-10) : 0;
}

#ifdef USE_WIFI
WifiAccess wifiAccess;
#endif

#ifdef USE_WIFI_WEB

#include "web-images.h"

////////////////////////////////
// List of available sequences by name and matching id
enum
{
    kMAX_FADE = 15,
    kMAX_DELAY = 500,
    kMIN_DELAY = 10,
    kMIN_BRI = 10,

    kMAX_ADJLOOP = 90000,
    kMIN_ADJLOOP = 500,
};

WMenuData mainMenu[] = {
    { "Logics", "/logics" },
    { "Setup", "/setup" }
};

WMenuData setupMenu[] = {
    { "Home", "/" },
    { "Logics", "/setuplogics" },
    { "Marcduino", "/marcduino" },
    { "WiFi", "/wifi" },
    { "Firmware", "/firmware" },
    { "Back", "/" }
};

WElement mainContents[] = {
    WVerticalMenu("menu", mainMenu, SizeOfArray(mainMenu)),
    rseriesSVG
};

WElement setupContents[] = {
    WVerticalMenu("setup", setupMenu, SizeOfArray(setupMenu)),
    rseriesSVG
};

String fldBoards[] = {
    "None",
    "RSeries 40 LED",
    "RSeries 40 LED Inverted",
    "C3PO 40 LED",
    "Naboo 48 LED"
};

String rldBoards[] = {
    "None",
    "Imahara 112 LED",
    "Imahara 112 LED Inverted",
    "DeathStar 96 LED",
    "DeathStar 96 LED StaggerOdd",
    "DeathStar 96 LED Inverted",
    "DeathStar 96 LED Inverted StaggerOdd",
    "Kenny 96 LED",
    "Naboo 96 LED"
};

String logicsSeq[] = {
#define LOGICENGINE_SEQ(nam, val) \
    BUILTIN_SEQ(nam, LogicEngineDefaults::val)
#define BUILTIN_SEQ(nam, val) \
    nam,

#include "logic-sequences.h"

#undef BUILTIN_SEQ
#undef LOGICENGINE_SEQ
};

unsigned logicsSeqNumber[] = {
#define LOGICENGINE_SEQ(nam, val) \
    BUILTIN_SEQ(nam, LogicEngineDefaults::val)
#define BUILTIN_SEQ(nam, val) \
    val,

#include "logic-sequences.h"

#undef BUILTIN_SEQ
#undef LOGICENGINE_SEQ
};

String logicsColors[] = {
    "Default",
    "Red",
    "Orange",
    "Yellow",
    "Green",
    "Cyan",
    "Blue",
    "Purple",
    "Magenta",
    "Pink"
};

bool sFLDChanged = true;
bool sRLDChanged = true;

int sFLDSequence;
int sRLDSequence;

String sFLDText = "";
String sRLDText = "";
String sFLDDisplayText;
String sRLDDisplayText;

int sFLDColor = LogicEngineRenderer::kDefault;
int sRLDColor = LogicEngineRenderer::kDefault;

int sFLDSpeedScale;
int sRLDSpeedScale;

int sFLDNumSeconds;
int sRLDNumSeconds;

/////////////////////////////////////////////////////////////////////////
// Web Interface for logic engine animation sequences
WElement logicsContents[] = {
    WSelect("Front Logic Sequence", "frontseq",
        logicsSeq, SizeOfArray(logicsSeq),
        []() { return sFLDSequence; },
        [](int val) { sFLDSequence = val; sFLDChanged = true; } ),
    WSelect("Front Color", "frontcolor",
        logicsColors, SizeOfArray(logicsColors),
        []() { return sFLDColor; },
        [](int val) { sFLDColor = val; sFLDChanged = true; } ),
    WSlider("Animation Speed", "fldspeed", 0, 9,
        []()->int { return sFLDSpeedScale; },
        [](int val) { sFLDSpeedScale = val; sFLDChanged = true; } ),
    WSlider("Number of seconds", "fldseconds", 0, 99,
        []()->int { return sFLDNumSeconds; },
        [](int val) { sFLDNumSeconds = val; sFLDChanged = true; } ),
    WTextField("Front Text:", "fronttext",
        []()->String { return sFLDText; },
        [](String val) { sFLDText = val; sFLDChanged = true; } ),
    WSelect("Rear Logic Sequence", "rearseq",
        logicsSeq, SizeOfArray(logicsSeq),
        []() { return sRLDSequence; },
        [](int val) { sRLDSequence = val; sRLDChanged = true; } ),
    WSelect("Rear Color", "rearcolor",
        logicsColors, SizeOfArray(logicsColors),
        []() { return sRLDColor; },
        [](int val) { sRLDColor = val; sRLDChanged = true; } ),
    WSlider("Animation Speed", "rldspeed", 0, 9,
        []()->int { return sRLDSpeedScale; },
        [](int val) { sRLDSpeedScale = val; sRLDChanged = true; } ),
    WSlider("Number of seconds", "rldseconds", 0, 99,
        []()->int { return sRLDNumSeconds; },
        [](int val) { sRLDNumSeconds = val; sRLDChanged = true; } ),
    WTextField("Rear Text:", "reartext",
        []()->String { return sRLDText; },
        [](String val) { sRLDText = val; sRLDChanged = true; } ),
    WButton("Run", "run", []() {
        if (sFLDChanged)
        {
            sFLDDisplayText = sFLDText;
            sFLDDisplayText.replace("\\n", "\n");
            FLD_selectSequence(logicsSeqNumber[sFLDSequence], (LogicEngineRenderer::ColorVal)sFLDColor, sFLDSpeedScale, sFLDNumSeconds);
            FLD_setTextMessage(sFLDDisplayText.c_str());
            sFLDChanged = false;
        }
        if (sRLDChanged)
        {
            sRLDDisplayText = sRLDText;
            sRLDDisplayText.replace("\\n", "\n");
            RLD_selectSequence(logicsSeqNumber[sRLDSequence], (LogicEngineRenderer::ColorVal)sRLDColor, sRLDSpeedScale, sRLDNumSeconds);
            RLD_setTextMessage(sRLDDisplayText.c_str());
            sRLDChanged = false;
        }
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    rseriesSVG
};

WElement audioContents[] = {
    WButton("Save", "save", []() {
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

String swBaudRates[] = {
    "2400",
    "9600",
};

int marcSerial1Baud;
int marcSerial2Baud;
bool marcSerialPass;
bool marcSerialEnabled;
bool marcWifiEnabled;
bool marcWifiSerialPass;

WElement marcduinoContents[] = {
    WSelect("Serial1 Baud Rate", "serial1baud",
        swBaudRates, SizeOfArray(swBaudRates),
        []() { return (marcSerial1Baud = (preferences.getInt(PREFERENCE_MARCSERIAL1, MARC_SERIAL1_BAUD_RATE)) == 2400) ? 0 : 1; },
        [](int val) { marcSerial1Baud = (val == 0) ? 2400 : 9600; } ),
    WSelect("Serial2 Baud Rate", "serial2baud",
        swBaudRates, SizeOfArray(swBaudRates),
        []() { return (marcSerial2Baud = (preferences.getInt(PREFERENCE_MARCSERIAL2, MARC_SERIAL2_BAUD_RATE)) == 2400) ? 0 : 1; },
        [](int val) { marcSerial2Baud = (val == 0) ? 2400 : 9600; } ),
    WCheckbox("Serial1 pass-through to Serial2", "serialpass",
        []() { return (marcSerialPass = (preferences.getBool(PREFERENCE_MARCSERIAL_PASS, MARC_SERIAL_PASS))); },
        [](bool val) { marcSerialPass = val; } ),
    WCheckbox("Marcduino on Serial1", "enabled",
        []() { return (marcSerialEnabled = (preferences.getBool(PREFERENCE_MARCSERIAL_ENABLED, MARC_SERIAL_ENABLED))); },
        [](bool val) { marcSerialEnabled = val; } ),
    WCheckbox("Marcduino on Wifi (port 2000)", "wifienabled",
        []() { return (marcWifiEnabled = (preferences.getBool(PREFERENCE_MARCWIFI_ENABLED, MARC_WIFI_ENABLED))); },
        [](bool val) { marcWifiEnabled = val; } ),
    WCheckbox("Marcduino Wifi pass-through to Serial2", "wifipass",
        []() { return (marcWifiSerialPass = (preferences.getBool(PREFERENCE_MARCWIFI_SERIAL_PASS, MARC_WIFI_SERIAL_PASS))); },
        [](bool val) { marcWifiSerialPass = val; } ),
    WButton("Save", "save", []() {
        preferences.putInt(PREFERENCE_MARCSERIAL1, marcSerial1Baud);
        preferences.putInt(PREFERENCE_MARCSERIAL2, marcSerial2Baud);
        preferences.putBool(PREFERENCE_MARCSERIAL_PASS, marcSerialPass);
        preferences.putBool(PREFERENCE_MARCSERIAL_ENABLED, marcSerialEnabled);
        preferences.putBool(PREFERENCE_MARCWIFI_ENABLED, marcWifiEnabled);
        preferences.putBool(PREFERENCE_MARCWIFI_SERIAL_PASS, marcWifiSerialPass);
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

WElement setupLogicsContents[] = {
    WSelect("Front Logic Engine Type", "fld",
        fldBoards, SizeOfArray(fldBoards),
        []() { return (int)sFLDType; },
        [](int val) { FLD_selectType((LogicEngineFLDType)val); } ),
    WSelect("Rear Logic Engine Type", "rld",
        rldBoards, SizeOfArray(rldBoards),
        []() { return (int)sRLDType; },
        [](int val) { RLD_selectType((LogicEngineRLDType)val); } ),
    WSlider("Front Brightness", "fldbri", 0, 255,
        []()->int { return sController.getSettings(0).fBri; },
        [](int val) { sController.changeSetting(0, 1, val); } ),
    WSlider("Front Hue", "fldhue", 0, 255,
        []()->int { return sController.getSettings(0).fHue; },
        [](int val) { sController.changeSetting(0, 2, val); } ),
    WSlider("Front Fade", "fldfad", 0, 255,
        []()->int { return sController.getSettings(0).fFade; },
        [](int val) { sController.changeSetting(0, 3, val); } ),
    WSlider("Front Pause", "fldpau", 0, 255,
        []()->int { return sController.getSettings(0).fDelay; },
        [](int val) { sController.changeSetting(0, 4, val); } ),
    WSlider("Front Palette", "fldpal", 0, LogicEngineDefaults::PAL_COUNT,
        []()->int { return sController.getSettings(0).fPalNum; },
        [](int val) { sController.changeSetting(0, 5, val); } ),
    WSlider("Rear Brightness", "rearbri", 0, 255,
        []()->int { return sController.getSettings(1).fBri; },
        [](int val) { sController.changeSetting(1, 1, val); } ),
    WSlider("Rear Hue", "rearhue", 0, 255,
        []()->int { return sController.getSettings(1).fHue; },
        [](int val) { sController.changeSetting(1, 2, val); } ),
    WSlider("Rear Fade", "rearfae", 0, 255,
        []()->int { return sController.getSettings(1).fFade; },
        [](int val) { sController.changeSetting(1, 3, val); } ),
    WSlider("Rear Pause", "rearpau", 0, 255,
        []()->int { return sController.getSettings(1).fDelay; },
        [](int val) { sController.changeSetting(1, 4, val); } ),
    WSlider("Rear Palette", "rearpal", 0, LogicEngineDefaults::PAL_COUNT,
        []()->int { return sController.getSettings(1).fPalNum; },
        [](int val) { sController.changeSetting(1, 5, val); } ),
    WButton("Save", "save", []() {
        preferences.putUChar(PREFERENCE_FLD, sFLDType);
        preferences.putUChar(PREFERENCE_RLD, sRLDType);
        sController.commit();
    }),
    WHorizontalAlign(),
    WButtonReload("Defaults", "default", []() {
        sController.restoreFactoryDefaults(0);
        sController.restoreFactoryDefaults(1);
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

String wifiSSID;
String wifiPass;
bool wifiAP;
bool wifiEnabled;

WElement wifiContents[] = {
    W1("WiFi Setup"),
    WCheckbox("WiFi Enabled", "enabled",
        []() { return (wifiEnabled = preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED)); },
        [](bool val) { wifiEnabled = val; } ),
    WCheckbox("Access Point", "apmode",
        []() { return (wifiAP = preferences.getBool(PREFERENCE_WIFI_AP, WIFI_ACCESS_POINT)); },
        [](bool val) { wifiAP = val; } ),
    WTextField("WiFi:", "wifi",
        []()->String { return (wifiSSID = preferences.getString(PREFERENCE_WIFI_SSID, WIFI_AP_NAME)); },
        [](String val) { wifiSSID = val; } ),
    WPassword("Password:", "password",
        []()->String { return (wifiPass = preferences.getString(PREFERENCE_WIFI_PASS, WIFI_AP_PASSPHRASE)); },
        [](String val) { wifiPass = val; } ),
    WButton("Save", "save", []() {
        DEBUG_PRINTLN("WiFi Changed");
        preferences.putBool(PREFERENCE_WIFI_ENABLED, wifiEnabled);
        preferences.putBool(PREFERENCE_WIFI_AP, wifiAP);
        preferences.putString(PREFERENCE_WIFI_SSID, wifiSSID);
        preferences.putString(PREFERENCE_WIFI_PASS, wifiPass);
        DEBUG_PRINTLN("Restarting");
        preferences.end();
        ESP.restart();
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

WElement firmwareContents[] = {
    W1("Firmware Setup"),
    WFirmwareFile("Firmware:", "firmware"),
    WFirmwareUpload("Reflash", "firmware"),
    WLabel("Current Firmware Build Date:", "label"),
    WLabel(__DATE__, "date"),
    WButton("Clear Prefs", "clear", []() {
        DEBUG_PRINTLN("Clear all preference settings");
        preferences.clear();
    }),
    WHorizontalAlign(),
    WButton("Reboot", "reboot", []() {
        DEBUG_PRINTLN("Rebooting");
        preferences.end();
        ESP.restart();
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

//////////////////////////////////////////////////////////////////

WPage pages[] = {
    WPage("/", mainContents, SizeOfArray(mainContents)),
      WPage("/logics", logicsContents, SizeOfArray(logicsContents)),
    WPage("/setup", setupContents, SizeOfArray(setupContents)),
      WPage("/audio", audioContents, SizeOfArray(audioContents)),
      WPage("/setuplogics", setupLogicsContents, SizeOfArray(setupLogicsContents)),
      WPage("/marcduino", marcduinoContents, SizeOfArray(marcduinoContents)),
      WPage("/wifi", wifiContents, SizeOfArray(wifiContents)),
      WPage("/firmware", firmwareContents, SizeOfArray(firmwareContents)),
        WUpload("/upload/firmware",
            [](Client& client)
            {
                if (Update.hasError())
                    client.println("HTTP/1.0 200 FAIL");
                else
                    client.println("HTTP/1.0 200 OK");
                client.println("Content-type:text/html");
                client.println("Vary: Accept-Encoding");
                client.println();
                client.println();
                client.stop();
                if (!Update.hasError())
                {
                    delay(1000);
                    preferences.end();
                    ESP.restart();
                }
            #ifdef USE_LOGICS
                FLD_selectSequence(LogicEngineDefaults::FAILURE);
                FLD_setTextMessage("Flash Fail");
                FLD_selectSequence(LogicEngineDefaults::TEXTSCROLLLEFT, LogicEngineRenderer::kRed, 1, 0);
                FLD_setEffectWidthRange(1.0);
                FLD_setEffectWidthRange(1.0);
            #endif
                otaInProgress = false;
            },
            [](WUploader& upload)
            {
                if (upload.status == UPLOAD_FILE_START)
                {
                    otaInProgress = true;
                    unmountFileSystems();
                #ifdef USE_AUDIO
                    audio.stopSong();
                #endif
                #ifdef USE_LOGICS
                    FLD_selectSequence(LogicEngineDefaults::NORMAL);
                    RLD_selectSequence(LogicEngineDefaults::NORMAL);
                    FLD_setEffectWidthRange(0);
                    RLD_setEffectWidthRange(0);
                #endif
                    Serial.printf("Update: %s\n", upload.filename.c_str());
                    if (!Update.begin(upload.fileSize))
                    {
                        //start with max available size
                        Update.printError(Serial);
                    }
                }
                else if (upload.status == UPLOAD_FILE_WRITE)
                {
                    float range = (float)upload.receivedSize / (float)upload.fileSize;
                    DEBUG_PRINTLN("Received: "+String(range*100)+"%");
                   /* flashing firmware to ESP*/
                    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                    {
                        Update.printError(Serial);
                    }
                #ifdef USE_LOGICS
                    FLD_setEffectWidthRange(range);
                    RLD_setEffectWidthRange(range);
                #endif
                }
                else if (upload.status == UPLOAD_FILE_END)
                {
                    DEBUG_PRINTLN("GAME OVER");
                    if (Update.end(true))
                    {
                        //true to set the size to the current progress
                        Serial.printf("Update Success: %u\nRebooting...\n", upload.receivedSize);
                    }
                    else
                    {
                        Update.printError(Serial);
                    }
                }
            })
};

WifiWebServer<10,SizeOfArray(pages)> webServer(pages, wifiAccess);
#endif

#ifdef USE_WIFI_MARCDUINO
WifiMarcduinoReceiver wifiMarcduinoReceiver(wifiAccess);
#endif

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
                Serial.println("Rebooting...");
                preferences.end();
                ESP.restart();
            }
        }
    }

#ifdef USE_WIFI_WEB
    wifiAccess.setNetworkCredentials(
        preferences.getString(PREFERENCE_WIFI_SSID, WIFI_AP_NAME),
        preferences.getString(PREFERENCE_WIFI_PASS, WIFI_AP_PASSPHRASE),
        preferences.getBool(PREFERENCE_WIFI_AP, WIFI_ACCESS_POINT),
        preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED));
#endif

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

    delay(200);

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

    SetupEvent::ready();

    // Setup button and trimpot controller
    sController.configure(FLD, RLD);
    sController.setWifiReset([]() {
        DEBUG_PRINTLN("WiFi Changed");
        preferences.putString(PREFERENCE_WIFI_SSID, WIFI_AP_NAME);
        preferences.putString(PREFERENCE_WIFI_PASS, WIFI_AP_PASSPHRASE);
        preferences.putBool(PREFERENCE_WIFI_AP, WIFI_ACCESS_POINT);
        preferences.putBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED);
        preferences.end();
        DEBUG_PRINTLN("Restarting");
        ESP.restart();
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
        preferences.end();
        DEBUG_PRINTLN("Restarting");
        ESP.restart();
    });

    Serial.print("Total heap:  "); Serial.println(ESP.getHeapSize());
    Serial.print("Free heap:   "); Serial.println(ESP.getFreeHeap());
    Serial.print("Total PSRAM: "); Serial.println(ESP.getPsramSize());
    Serial.print("Free PSRAM:  "); Serial.println(ESP.getFreePsram());

#ifdef USE_AUDIO
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(12); // 0...21
#endif

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

#ifdef USE_AUDIO
    RLD_setPeakValueProvider(audioPeakValue);
    FLD_setPeakValueProvider(audioPeakValue);
#endif
    RLD_setLogicEffectSelector(CustomLogicEffectSelector);
    FLD_setLogicEffectSelector(CustomLogicEffectSelector);

#ifdef LIVE_STREAM
    if (udp.listen(1234)) {
    }
#endif

#ifdef USE_AUDIO
    playRandom();
    //audio.connecttohost("http://macslons-irish-pub-radio.com/media.asx");
#endif
    xTaskCreatePinnedToCore(
          audioLoopTask,
          "Audio",
          10000,    // shrink stack size?
          NULL,
          1,
          &audioTask,
          0);
#ifdef USE_AUDIO
    // Setup audio processing filter for LEDs
    audio.setSampleFilter([](unsigned numBits, unsigned numChannels, const int16_t* samples, unsigned sampleCount) {
        audioBitmap.processSamples(numBits, numChannels, samples, sampleCount);
    });
#endif
}

////////////////

MARCDUINO_ACTION(DirectCommand, ~RT, ({
    // Direct ReelTwo command
    CommandEvent::process(Marcduino::getCommand());
}))

////////////////

MARCDUINO_ACTION(WifiByeBye, #WIBYE, ({
    bool wifiEnabled = preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED);
    if (wifiEnabled)
    {
        preferences.putBool(PREFERENCE_WIFI_ENABLED, false);
        preferences.end();
        DEBUG_PRINTLN("Enabling WiFi. Restarting");
        ESP.restart();
    }
}))

////////////////

MARCDUINO_ACTION(WifiHiHi, #WIHI, ({
    bool wifiEnabled = preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED);
    if (!wifiEnabled)
    {
        preferences.putBool(PREFERENCE_WIFI_ENABLED, true);
        preferences.end();
        DEBUG_PRINTLN("Disabling WiFi. Restarting");
        ESP.restart();
    }
}))

////////////////

void loop()
{
#ifdef USE_OTA
    if (otaInProgress)
        ArduinoOTA.handle();
#endif
    if (!otaInProgress)
    {
        AnimatedEvent::process();
    }
#ifdef USE_WIFI_WEB
    if (preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED))
    {
        webServer.handle();
    }
#endif
}

#ifdef USE_AUDIO
// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     "); Serial.println(info);
}
void audio_id3image(File& file, const size_t pos, const size_t size){
    Serial.print("id3image     "); Serial.printf("at pos: %u, length: %u\n", pos, size);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
    playRandom();
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    RLDText = info;
    RLD_selectScrollTextLeft(RLDText.c_str());
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}
#endif
