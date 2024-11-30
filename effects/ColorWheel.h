static bool LogicEffectColorWheel(LogicEngineRenderer& r)
{
    //overall brightness , 0=black, 255=crazy super bright
    static constexpr uint8_t frontBri = 150;
    //number of loops that a tween color is held for
    static constexpr uint8_t frontFade = 2;
    //number of loops that a key color is held for
    static constexpr unsigned int frontDelay = 350;
    static constexpr uint8_t TWEENS = 20;
    static constexpr uint8_t fKeyColors[4][4] =  {
        {   0,   0,   0,   0},
        { 155, 155, 155, 100},
        {   0,   0, 240,   8},
        {   0,   0, 255,  16}
    };
    class ColorWheel : public LogicEffectObject
    {
    public:
        struct LEDStatus {
          uint8_t colorNum;
          uint8_t nextColorNum;
          uint8_t tweenNum;
          unsigned int colorPause;
        };
        LEDStatus* fLEDstatus;
        LogicEngineRenderer& fR;
        int pixel;

        ColorWheel(LogicEngineRenderer& r) :
            fLEDstatus(new LEDStatus[r.count()]),
            fR(r)
        {
            //go through all the front LEDs and assign an initial color
            for (unsigned LEDnum = 0; LEDnum < r.count(); LEDnum++) { 
                uint8_t color = random(0, 3);
                uint8_t nextColor;
                do {
                    nextColor = random(0, 3);
                } while (nextColor == color);
                fLEDstatus[LEDnum].colorNum = color;
                fLEDstatus[LEDnum].nextColorNum = nextColor;
                fLEDstatus[LEDnum].tweenNum = random(0, TWEENS);
                fLEDstatus[LEDnum].colorPause = random(0, 256);
            }  
        }

        virtual ~ColorWheel() override {
            delete[] fLEDstatus;
        }

        //here's a helper function to figure out rgbw values of a color that might be between two of our RGBW key color components
        void updatePixel(int x, int y, unsigned LEDnum) {
            //led's primary color is currently one of the keys : fKeyColors[fLEDstatus[LEDnum].colorNum][RGBW]
            //led's next color is also going to be one of the keys : fKeyColors[fLEDstatus[LEDnum].nextColorNum][RGBW]
            //we could be somewhere between those two colors if this is >0 : fLEDstatus[LEDnum].tweenNum
            if (fLEDstatus[LEDnum].colorPause == 0) {
                fLEDstatus[LEDnum].tweenNum++;
                if (fLEDstatus[LEDnum].tweenNum == TWEENS) {
                    //we've reached the max tweens, so we need to change the primary color and randomly choose a new nextColor
                    fLEDstatus[LEDnum].colorNum = fLEDstatus[LEDnum].nextColorNum;
                    // randomly select the next color that this LED will fade to
                    fLEDstatus[LEDnum].nextColorNum = random(0, 4);
                    while (fLEDstatus[LEDnum].colorNum == fLEDstatus[LEDnum].nextColorNum) {
                        //this makes sure the next color isn't the same as the current color
                        fLEDstatus[LEDnum].nextColorNum = random(0, 4);
                    }
                    //restart the tween count
                    fLEDstatus[LEDnum].tweenNum = 0;
                    //we randomize this hold time on the key color to keep things interesting - this causes the patterns to evolve over time
                    fLEDstatus[LEDnum].colorPause = random(0, frontDelay);
                }
                uint8_t wipColor[] = { 0, 0, 0, 0 };
                if (fLEDstatus[LEDnum].tweenNum == 0) {
                    //we're at the initial color, so no tween calculation needed
                    for (uint8_t RGBW = 0; RGBW < SizeOfArray(wipColor); RGBW++) {
                        wipColor[RGBW] = fKeyColors[fLEDstatus[LEDnum].colorNum][RGBW];
                    }
                } else {
                    //we're in the tween zone, somewhere between colorNum and nextColorNum
                    //lets figure out wipColor values based on those two colors and our tween
                    for (uint8_t RGBW = 0; RGBW < SizeOfArray(wipColor); RGBW++) {
                        wipColor[RGBW] = map(fLEDstatus[LEDnum].tweenNum, 0, TWEENS,
                            fKeyColors[fLEDstatus[LEDnum].colorNum][RGBW],
                            fKeyColors[fLEDstatus[LEDnum].nextColorNum][RGBW]);
                    }
                    fLEDstatus[LEDnum].colorPause = frontFade; //tween colors don't hold as long as a key color
                }
                //now set the actual neopixel color for this led...
                fR.setPixelRGBW(x, y,
                    map(wipColor[0], 0, 255, 0, frontBri),
                    map(wipColor[1], 0, 255, 0, frontBri),
                    map(wipColor[2], 0, 255, 0, frontBri),
                    map(wipColor[3], 0, 255, 0, frontBri) 
                );
            } else {
                //we've done all the things, so reduce the pause loops by 1
                fLEDstatus[LEDnum].colorPause--;
            }
        }
    };

    if (r.hasEffectChanged())
    {
        r.setEffectObject(new ColorWheel(r));
        r.clear();
    }
    ColorWheel* wheel = (ColorWheel*)r.getEffectObject();
    auto h = r.height();
    auto w = r.width();
    int count = 0;
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++, count++) {
            wheel->updatePixel(x, y, count);
        }
    }
    return true;
}
