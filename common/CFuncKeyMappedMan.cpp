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
    // Returned struct is 5 bytes (packed); follow MSVC __thiscall calling
    // convention: caller-allocated return-slot pointer passed as hidden
    // first arg in ECX is NOT what the game uses here — small POD returns
    // are returned in EDX:EAX on x86. Confirm via decompile if the
    // generated thunk crashes; fall back to pass-by-reference if needed.
    return reinterpret_cast<FUNCKEY_MAPPED(__fastcall *)(
        CFuncKeyMappedMan *, void *, int)>(
        C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED)(this, nullptr, vk);
}
