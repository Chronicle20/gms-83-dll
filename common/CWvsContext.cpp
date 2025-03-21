#include "pch.h"
#include "memory_map.h"

CWvsContext *CWvsContext::GetInstance() {
    return reinterpret_cast<CWvsContext *>(*(void **) C_WVS_CONTEXT_GET_INSTANCE);
}