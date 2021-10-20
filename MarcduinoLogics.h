////////////////

MARCDUINO_ACTION(FLDNormalSequence, @1T1, ({
    FLD_selectSequence(LogicEngineRenderer::NORMAL);
}))

////////////////

MARCDUINO_ACTION(FLDFlashSequence, @1T2, ({
    FLD_selectSequence(LogicEngineRenderer::FLASHCOLOR);
}))

////////////////

MARCDUINO_ACTION(FLDAlarmSequence, @1T3, ({
    FLD_selectSequence(LogicEngineRenderer::ALARM);
}))

////////////////

MARCDUINO_ACTION(FLDFailureSequence, @1T4, ({
    FLD_selectSequence(LogicEngineRenderer::FAILURE);
}))

////////////////

MARCDUINO_ACTION(FLDScreamLogicsSequence, @1T5, ({
    FLD_selectSequence(LogicEngineRenderer::REDALERT);
}))

////////////////

MARCDUINO_ACTION(FLDLeiaLogicsSequence, @1T6, ({
    FLD_selectSequence(LogicEngineRenderer::LEIA);
}))

////////////////

MARCDUINO_ACTION(FLDMarchSequence, @1T11, ({
    FLD_selectSequence(LogicEngineRenderer::MARCH);
}))

////////////////

MARCDUINO_ACTION(RLDNormalSequence, @2T1, ({
    RLD_selectSequence(LogicEngineRenderer::NORMAL);
}))

////////////////

MARCDUINO_ACTION(RLDFlashSequence, @2T2, ({
    RLD_selectSequence(LogicEngineRenderer::FLASHCOLOR);
}))

////////////////

MARCDUINO_ACTION(RLDAlarmSequence, @2T3, ({
    RLD_selectSequence(LogicEngineRenderer::ALARM);
}))

////////////////

MARCDUINO_ACTION(RLDFailureSequence, @2T4, ({
    RLD_selectSequence(LogicEngineRenderer::FAILURE);
}))

////////////////

MARCDUINO_ACTION(RLDScreamLogicsSequence, @2T5, ({
    RLD_selectSequence(LogicEngineRenderer::REDALERT);
}))

////////////////

MARCDUINO_ACTION(RLDLeiaLogicsSequence, @2T6, ({
    RLD_selectSequence(LogicEngineRenderer::LEIA);
}))

////////////////

MARCDUINO_ACTION(RLDMarchSequence, @2T11, ({
    RLD_selectSequence(LogicEngineRenderer::MARCH);
}))

////////////////

MARCDUINO_ACTION(NormalSequence, @0T1, ({
    FLD_selectSequence(LogicEngineRenderer::NORMAL);
    RLD_selectSequence(LogicEngineRenderer::NORMAL);
}))

////////////////

MARCDUINO_ACTION(FlashSequence, @0T2, ({
    FLD_selectSequence(LogicEngineRenderer::FLASHCOLOR);
    RLD_selectSequence(LogicEngineRenderer::FLASHCOLOR);
}))

////////////////

MARCDUINO_ACTION(AlarmSequence, @0T3, ({
    FLD_selectSequence(LogicEngineRenderer::ALARM);
    RLD_selectSequence(LogicEngineRenderer::ALARM);
}))

////////////////

MARCDUINO_ACTION(FailureSequence, @0T4, ({
    FLD_selectSequence(LogicEngineRenderer::FAILURE);
    RLD_selectSequence(LogicEngineRenderer::FAILURE);
}))

////////////////

MARCDUINO_ACTION(ScreamLogicsSequence, @0T5, ({
    FLD_selectSequence(LogicEngineRenderer::REDALERT);
    RLD_selectSequence(LogicEngineRenderer::REDALERT);
}))

////////////////

MARCDUINO_ACTION(LeiaLogicsSequence, @0T6, ({
    FLD_selectSequence(LogicEngineRenderer::LEIA);
    RLD_selectSequence(LogicEngineRenderer::LEIA);
}))

////////////////

MARCDUINO_ACTION(MarchSequence, @0T11, ({
    FLD_selectSequence(LogicEngineRenderer::MARCH);
    RLD_selectSequence(LogicEngineRenderer::MARCH);
}))

////////////////

static char sMTFLDText[32];
static char sMBFLDText[32];
static char sMFLDText[65];
static char sMRLDText[64];

MARCDUINO_ACTION(TFLDScrollTextLeft, @1M, ({
    strcpy(sMTFLDText, Marcduino::getCommand());
    strcpy(sMFLDText, sMTFLDText);
    strcat(sMFLDText, "\n");
    strcat(sMFLDText, sMBFLDText);
    if (FLD != nullptr)
        FLD_selectScrollTextLeft(sMFLDText, FLD->randomColor());
}))

////////////////

MARCDUINO_ACTION(BFLDScrollTextLeft, @2M, ({
    strcpy(sMBFLDText, Marcduino::getCommand());
    strcpy(sMFLDText, sMTFLDText);
    strcat(sMFLDText, "\n");
    strcat(sMFLDText, sMBFLDText);
    if (FLD != nullptr)
        FLD_selectScrollTextLeft(sMFLDText, FLD->randomColor());
}))

////////////////

MARCDUINO_ACTION(RLDScrollTextLeft, @3M, ({
    strcpy(sMRLDText, Marcduino::getCommand());
    if (RLD != nullptr)
        RLD_selectScrollTextLeft(sMRLDText, RLD->randomColor());
}))

////////////////

MARCDUINO_ACTION(TFLDTextLatin, @1P60, ({
    FLD_setEffectFontNum(0);
}))

////////////////

MARCDUINO_ACTION(BFLDTextLatin, @2P60, ({
    FLD_setEffectFontNum(0);
}))

////////////////

MARCDUINO_ACTION(RLDTextLatin, @3P60, ({
    RLD_setEffectFontNum(0);
}))

////////////////

MARCDUINO_ACTION(TFLDTextAurabesh, @1P61, ({
    FLD_setEffectFontNum(1);
}))

////////////////

MARCDUINO_ACTION(BFLDTextAurabesh, @2P61, ({
    FLD_setEffectFontNum(1);
}))

////////////////

MARCDUINO_ACTION(RLDTextAurabesh, @3P61, ({
    RLD_setEffectFontNum(1);
}))

////////////////
