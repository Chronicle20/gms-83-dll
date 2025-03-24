#include "pch.h"
#include "CLogin.h"

void CLogin::Update() {
    ((VOID(_fastcall * )(CLogin * , PVOID))
    C_LOGIN_UPDATE)(this, nullptr);
}
