#include "pch.h"

void CQuickslotKeyMappedMan::CreateInstance() {
    Log("CQuickslotKeyMappedMan::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_QUICKSLOT_KEY_MAPPED_MAN)();
}