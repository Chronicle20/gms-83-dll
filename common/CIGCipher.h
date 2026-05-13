#pragma once

class CIGCipher {
  public:
    static unsigned int innoHash(unsigned char* pSrc, int nLen, unsigned int* pdwKey);
};
