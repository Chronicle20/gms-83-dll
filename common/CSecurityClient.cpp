#include "pch.h"

typedef VOID(__cdecl *_CSecurityClient__CreateInstance_t)();
_CSecurityClient__CreateInstance_t _CSecurityClient__CreateInstance = reinterpret_cast<_CSecurityClient__CreateInstance_t>(0x00ADCE67);

// void __stdcall TSingleton<CSecurityClient>::CreateInstance()
void CSecurityClient::CreateInstance() {
    _CSecurityClient__CreateInstance();
}

CSecurityClient *CSecurityClient::GetInstance() {
    return reinterpret_cast<CSecurityClient *>(*(void **) 0x00CD5C44);
}