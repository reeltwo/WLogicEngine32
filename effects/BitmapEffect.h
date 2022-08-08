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
