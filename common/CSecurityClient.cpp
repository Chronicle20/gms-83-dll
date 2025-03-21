#include "pch.h"
#include "memory_map.h"

void CSecurityClient::CreateInstance() {
    ((VOID * *(_fastcall * )())
            C_SECURITY_CLIENT_CREATE_INSTANCE)();
}
