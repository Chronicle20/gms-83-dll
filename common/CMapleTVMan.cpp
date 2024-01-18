#include "pch.h"

void CMapleTVMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x009F9F87)();
}

CMapleTVMan *CMapleTVMan::GetInstance() {
    return reinterpret_cast<CMapleTVMan *>(*(void **) 0x00BED76C);
}

void CMapleTVMan::Init() {
    ((VOID(_fastcall * )(CMapleTVMan * , PVOID))
    0x00636F4E)(this, NULL);
}
