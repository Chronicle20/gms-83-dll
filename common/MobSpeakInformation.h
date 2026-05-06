#pragma once

class MobSpeakInformation : ZRefCounted {
    int nAction;
    int nHP;
    int nMP;
    int nProb;
    int nWidth;
    ZArray<ZXString<char> > asSpeech;
};
