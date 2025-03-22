#include "pch.h"

void CActionMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    C_ACTION_MAN_CREATE_INSTANCE_ADDR)();
}

CActionMan *CActionMan::GetInstance() {
    return reinterpret_cast<CActionMan *>(*(void **) C_ACTION_MAN_GET_INSTANCE_ADDR);
}

void CActionMan::Init() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    C_ACTION_MAN_INIT)(this, NULL);
}

void CActionMan::SweepCache() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    C_ACTION_MAN_SWEEP_CACHE)(this, NULL);
}