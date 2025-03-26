#include "pch.h"

void CSecurityClient::CreateInstance() {
    reinterpret_cast<void (__fastcall *)()>(C_SECURITY_CLIENT_CREATE_INSTANCE)();
}

CSecurityClient *CSecurityClient::GetInstance() {
    return reinterpret_cast<CSecurityClient *>(*reinterpret_cast<void **>(C_SECURITY_CLIENT_GET_INSTANCE));
}