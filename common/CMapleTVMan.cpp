#include "pch.h"

void CMapleTVMan::CreateInstance() {
    Log("CMapleTVMan::CreateInstance");
    reinterpret_cast<void (__fastcall *)()>(C_MAPLE_TV_MAN_CREATE_INSTANCE)();
}

CMapleTVMan *CMapleTVMan::GetInstance() {
    return reinterpret_cast<CMapleTVMan *>(*reinterpret_cast<void **>(C_MAPLE_TV_MAN_GET_INSTANCE));
}


#if defined(REGION_GMS)

void CMapleTVMan::Init() {
    Log("CMapleTVMan::Init");
    reinterpret_cast<void (__fastcall *)(CMapleTVMan *, void *)>(C_MAPLE_TV_MAN_INIT)(this, nullptr);
}

#elif defined(REGION_JMS)
void CMapleTVMan::Init(int something, int somethingElse) {
    Log("CMapleTVMan::Init");
    reinterpret_cast<void(__fastcall*)(CMapleTVMan*, void*, int, int)>(
        C_MAPLE_TV_MAN_INIT)(this, nullptr, something, somethingElse);
}
#endif