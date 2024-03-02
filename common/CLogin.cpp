#include "pch.h"
#include "CLogin.h"

typedef VOID(__thiscall *_CLogin__Update_t)(CLogin *pThis);
_CLogin__Update_t _CLogin__Update = reinterpret_cast<_CLogin__Update_t>(0x0066BC8C);

void CLogin::Update() {
    _CLogin__Update(nullptr);
}
