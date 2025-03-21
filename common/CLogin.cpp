#include "pch.h"
#include "memory_map.h"
#include "CLogin.h"

void CLogin::Update() {
    ((VOID(_fastcall * )(CLogin * , PVOID))
    C_LOGIN_UPDATE)(this, NULL);
}
