#include "pch.h"

CFuncKeyMappedMan::CFuncKeyMappedMan() {
    Log("CFuncKeyMappedMan::CFuncKeyMappedMan");
    reinterpret_cast<void (__fastcall *)(CFuncKeyMappedMan *, void *)>(C_FUNC_KEY_MAPPED_MAN)(this, nullptr);
};

void CFuncKeyMappedMan::CreateInstance() {
    Log("CFuncKeyMappedMan::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE)();
}

CFuncKeyMappedMan *CFuncKeyMappedMan::GetInstance() {
    return *reinterpret_cast<CFuncKeyMappedMan **>(
        C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR);
}

FUNCKEY_MAPPED CFuncKeyMappedMan::FuncKeyMapped(int vk) {
    // v83.1 has NO discrete CFuncKeyMappedMan::FuncKeyMapped function -- the
    // compiler inlined every call site as `*(instance + 4 + 5*vk)`. We mirror
    // that by indexing the member array directly. `vk` here is a func-key
    // slot index (0..88 GMS / 0..93 JMS); callers must pass a valid index.
    return m_aFuncKeyMapped[vk];
}
