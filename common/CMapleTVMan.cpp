#include "pch.h"

void CMapleTVMan::CreateInstance() {
    Log("CMapleTVMan::CreateInstance");
    ((VOID * *(_fastcall * )())
    C_MAPLE_TV_MAN_CREATE_INSTANCE)();
}

CMapleTVMan *CMapleTVMan::GetInstance() {
    return reinterpret_cast<CMapleTVMan *>(*(void **) C_MAPLE_TV_MAN_GET_INSTANCE);
}


#if defined(REGION_GMS)
void CMapleTVMan::Init() {
    Log("CMapleTVMan::Init");
    ((VOID(_fastcall * )(CMapleTVMan * , PVOID))
    C_MAPLE_TV_MAN_INIT)(this, nullptr);
}
#elif defined(REGION_JMS)
void CMapleTVMan::Init(int something, int somethingElse) {
    Log("CMapleTVMan::Init");
    ((VOID(_fastcall * )(CMapleTVMan * , PVOID, int something, int somethingElse))
    C_MAPLE_TV_MAN_INIT)(this, nullptr, something, somethingElse);
}
#endif