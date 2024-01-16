#include "CWvsContext.h"

CWvsContext *CWvsContext::GetInstance() {
    return reinterpret_cast<CWvsContext *>(*(void **) 0x00BE7918);
}