#include "pch.h"
#include "CLogin.h"

void CLogin::Update() {
    reinterpret_cast<void (__fastcall *)(CLogin *, void *)>(C_LOGIN_UPDATE)(this, nullptr);
}
