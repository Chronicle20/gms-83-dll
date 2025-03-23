#include "pch.h"

void CSecurityClient::CreateInstance() {
    ((VOID * *(_fastcall * )())
            C_SECURITY_CLIENT_CREATE_INSTANCE)();
}

CSecurityClient *CSecurityClient::GetInstance() {
    return reinterpret_cast<CSecurityClient *>(*(void **) C_SECURITY_CLIENT_GET_INSTANCE);
}