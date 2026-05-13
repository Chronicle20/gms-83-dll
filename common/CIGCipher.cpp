#include "pch.h"

unsigned int CIGCipher::innoHash(unsigned char *pSrc, int nLen, unsigned int *pdwKey) {
    return reinterpret_cast<unsigned int (__cdecl *)(unsigned char *, int, unsigned int *)>(
            C_IG_CIPHER_INNO_HASH)(pSrc, nLen, pdwKey);
}
