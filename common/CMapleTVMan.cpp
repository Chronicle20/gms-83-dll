#include "pch.h"

void CMapleTVMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00A8E5F3)();
}

CMapleTVMan *CMapleTVMan::GetInstance() {
    return reinterpret_cast<CMapleTVMan *>(*(void **) 0x00CA02AC);
}

void CMapleTVMan::Init() {
    ((VOID(_fastcall * )(CMapleTVMan * , PVOID))
    0x0066FFBD)(this, nullptr);
}
