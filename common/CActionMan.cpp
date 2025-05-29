#include "pch.h"

void CActionMan::CreateInstance() {
    Log("CActionMan::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_ACTION_MAN_CREATE_INSTANCE_ADDR)();
}

CActionMan *CActionMan::GetInstance() {
    return reinterpret_cast<CActionMan *>(*(void **) C_ACTION_MAN_INSTANCE_ADDR);
}

void CActionMan::Init() {
    Log("CActionMan::Init");
    reinterpret_cast<void (__fastcall *)(CActionMan *, void *)>(C_ACTION_MAN_INIT)(this, nullptr);
}

void CActionMan::SweepCache() {
    // High volume call
    reinterpret_cast<void (__fastcall *)(CActionMan *, void *)>(C_ACTION_MAN_SWEEP_CACHE)(this, nullptr);
}