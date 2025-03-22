#include "pch.h"


unsigned int CIOBufferManipulator::Decode2(unsigned short *n, const char *pSrc, unsigned int uRemain) {
    unsigned int result = 2;
    if (uRemain < 2) {
        throw std::invalid_argument("uRemain");
    }
    *n = *(unsigned short *)pSrc;
    return result;
}

unsigned int CIOBufferManipulator::DecodeStr(ZXString<char> *s, const char *pSrc, unsigned int uRemain) {
    int v3;
    int result;
    if (uRemain < 2) {
        throw std::invalid_argument("uRemain");
    }
    v3 = *pSrc;
    result = v3 + 2;
    if (uRemain < v3 + 2) {
        throw std::invalid_argument("uRemain");
    }
    s->Assign(pSrc + 2, v3);
    return result;
}

unsigned int CIOBufferManipulator::Decode4(unsigned int *n, const char *pSrc, unsigned int uRemain) {
    unsigned int result = 4;
    if (uRemain < 4) {
        throw std::invalid_argument("uRemain");
    }
    *n = *(unsigned int *)pSrc;
    return result;
}

unsigned int CIOBufferManipulator::Decode1(unsigned char *n, const char *pSrc, unsigned int uRemain) {
    unsigned int result = 1;
    if (!uRemain) {
        throw std::invalid_argument("uRemain");
    }
    *n = *pSrc;
    return result;
}