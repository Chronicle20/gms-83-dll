#include "pch.h"

void CActionMan::CreateInstance() {
    Log("CActionMan::CreateInstance");
    ((VOID * *(_fastcall * )())
    C_ACTION_MAN_CREATE_INSTANCE_ADDR)();
}

CActionMan *CActionMan::GetInstance() {
    return reinterpret_cast<CActionMan *>(*(void **) C_ACTION_MAN_GET_INSTANCE_ADDR);
}

void CActionMan::Init() {
    Log("CActionMan::Init");
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    C_ACTION_MAN_INIT)(this, NULL);
}

void CActionMan::SweepCache() {
    // High volume call
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    C_ACTION_MAN_SWEEP_CACHE)(this, NULL);
}