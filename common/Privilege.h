#pragma once

struct Privilege {
    unsigned int tEnd;
    unsigned __int8 bType;
    // 3 bytes of padding before nIndex (compiler-inserted)
    int nIndex;
    unsigned int dwIncExpRate;
    unsigned int dwIncDropRate;
};
