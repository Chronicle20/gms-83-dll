#include "pch.h"

CWvsContext *CWvsContext::GetInstance() {
    return reinterpret_cast<CWvsContext*>(*reinterpret_cast<void**>(C_WVS_CONTEXT_INSTANCE_ADDR));
}