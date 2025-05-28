#include "pch.h"

void CSecurityClient::CreateInstance() {
    Log("CSecurityClient::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_SECURITY_CLIENT_CREATE_INSTANCE)();
}

CSecurityClient *CSecurityClient::GetInstance() {
    return reinterpret_cast<CSecurityClient *>(*reinterpret_cast<void **>(C_SECURITY_CLIENT_INSTANCE_ADDR));
}