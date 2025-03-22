#include "pch.h"

void CSecurityClient::CreateInstance() {
    ((VOID * *(_fastcall * )())
            C_SECURITY_CLIENT_CREATE_INSTANCE)();
}
