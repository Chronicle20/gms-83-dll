#include "pch.h"
#include "memory_map.h"

void CMapleTVMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    C_MAPLE_TV_MAN_CREATE_INSTANCE)();
}

CMapleTVMan *CMapleTVMan::GetInstance() {
    return reinterpret_cast<CMapleTVMan *>(*(void **) C_MAPLE_TV_MAN_GET_INSTANCE);
}

void CMapleTVMan::Init() {
    ((VOID(_fastcall * )(CMapleTVMan * , PVOID))
    C_MAPLE_TV_MAN_INIT)(this, NULL);
}
