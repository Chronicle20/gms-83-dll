#include "ZFatalSection.h"
#include "ZRefCounted.h"

template<typename T>
class TemporaryStatBase : ZRefCounted {
    int m_value;
    int m_reason;
    unsigned int m_tLastUpdated;
    ZFatalSection m_lock;
};
