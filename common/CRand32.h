#include "ZFatalSection.h"

struct CRand32 {
    unsigned int m_s1;
    unsigned int m_s2;
    unsigned int m_s3;
    unsigned int m_past_s1;
    unsigned int m_past_s2;
    unsigned int m_past_s3;
    ZFatalSection m_lock;
};
