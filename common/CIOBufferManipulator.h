class CIOBufferManipulator {
public:
    static unsigned int Decode2(unsigned short *n, const char *pSrc, unsigned int uRemain);

    static unsigned int DecodeStr(ZXString<char> *s, const char *pSrc, unsigned int uRemain);

    static unsigned int Decode4(unsigned int *n, const char *pSrc, unsigned int uRemain);

    static unsigned int Decode1(unsigned char *n, const char *pSrc, unsigned int uRemain);
};
