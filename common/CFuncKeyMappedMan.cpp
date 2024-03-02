#include "pch.h"

typedef VOID(__cdecl *_CFuncKeyMappedMan__CreateInstance_t)();
_CFuncKeyMappedMan__CreateInstance_t _CFuncKeyMappedMan__CreateInstance = reinterpret_cast<_CFuncKeyMappedMan__CreateInstance_t>(0x00ADCDBD);

void CFuncKeyMappedMan::CreateInstance() {
    _CFuncKeyMappedMan__CreateInstance();
}
