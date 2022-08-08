///////////////////////////////////////////////////////////////////////////////
//
// Main screen shows current dome position if dome position changes.
// Push button will activate SelectScreen
// 
///////////////////////////////////////////////////////////////////////////////

static const char* sSequenceMenu[] = {
    "Normal",
    "Flash",
    "Alarm",
    "Failure",
    "Scream",
    "Leia",
    "March",
    "Fire",
    "Plasma",
    "Fractal",
    "Fade And\nScroll"
};

class SequenceScreen : public MenuScreen
{
public:
    enum {
        kNormal,
        kFlash,
        kAlarm,
        kFailure,
        kScream,
        kLeia,
        kMarch,
        kFire,
        kPlasma,
        kFractal,
        kFadeAndScroll
    };
    SequenceScreen() :
        MenuScreen(kSequenceScreen, sSequenceMenu, SizeOfArray(sSequenceMenu))
    {}

    virtual void buttonInReleased() override
    {
        switch (fCurrentItem)
        {
            case kNormal:
                FLD_selectSequence(LogicEngineRenderer::NORMAL);
                RLD_selectSequence(LogicEngineRenderer::NORMAL);
                break;
            case kFlash:
                FLD_selectSequence(LogicEngineRenderer::FLASHCOLOR);
                RLD_selectSequence(LogicEngineRenderer::FLASHCOLOR);
                break;
            case kAlarm:
                FLD_selectSequence(LogicEngineRenderer::ALARM);
                RLD_selectSequence(LogicEngineRenderer::ALARM);
                break;
            case kFailure:
                FLD_selectSequence(LogicEngineRenderer::FAILURE);
                RLD_selectSequence(LogicEngineRenderer::FAILURE);
                break;
            case kScream:
                FLD_selectSequence(LogicEngineRenderer::REDALERT);
                RLD_selectSequence(LogicEngineRenderer::REDALERT);
                break;
            case kLeia:
                FLD_selectSequence(LogicEngineRenderer::LEIA);
                RLD_selectSequence(LogicEngineRenderer::LEIA);
                break;
            case kMarch:
                FLD_selectSequence(LogicEngineRenderer::MARCH);
                RLD_selectSequence(LogicEngineRenderer::MARCH);
                break;
            case kFire:
                FLD_selectSequence(LogicEngineRenderer::FIRE);
                RLD_selectSequence(LogicEngineRenderer::FIRE);
                break;
            case kPlasma:
                FLD_selectSequence(PLASMA);
                RLD_selectSequence(PLASMA);
                break;
            case kFractal:
                FLD_selectSequence(FRACTAL);
                RLD_selectSequence(FRACTAL);
                break;
            case kFadeAndScroll:
                FLD_selectSequence(FADEANDSCROLL);
                RLD_selectSequence(FADEANDSCROLL);
                break;
        }
    }

#ifdef USE_DROID_REMOTE
    virtual void buttonLeftPressed(bool repeat) override
    {
        if (remoteEnabled)
        {
        #ifdef USE_SMQ
            if (SMQ::sendTopic("EXIT", "Remote"))
            {
                SMQ::sendString("addr", SMQ::getAddress());
                SMQ::sendEnd();
                sDisplay.setEnabled(false);
            }
        #endif
        }
    }
#endif
};

///////////////////////////////////////////////////////////////////////////////
//
// Instantiate the screen
//
///////////////////////////////////////////////////////////////////////////////

SequenceScreen sSequenceScreen;
