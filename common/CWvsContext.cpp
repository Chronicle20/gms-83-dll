#include "pch.h"

CWvsContext *CWvsContext::GetInstance() {
    return reinterpret_cast<CWvsContext *>(*(void **) C_WVS_CONTEXT_GET_INSTANCE);
}