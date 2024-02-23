#include "pch.h"

CWvsContext *CWvsContext::GetInstance() {
    return reinterpret_cast<CWvsContext *>(*(void **) 0x00C9A008);
}