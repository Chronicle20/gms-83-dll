#include "pch.h"

void CFuncKeyMappedMan::CreateInstance() {
    Log("CFuncKeyMappedMan::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE)();
}
