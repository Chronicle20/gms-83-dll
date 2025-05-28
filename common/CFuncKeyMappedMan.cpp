#include "pch.h"

CFuncKeyMappedMan::CFuncKeyMappedMan() {
    Log("CFuncKeyMappedMan::CFuncKeyMappedMan");
    reinterpret_cast<void (__fastcall *)(CFuncKeyMappedMan *, void *)>(C_FUNC_KEY_MAPPED_MAN)(this, nullptr);
};

void CFuncKeyMappedMan::CreateInstance() {
    Log("CFuncKeyMappedMan::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE)();
}
