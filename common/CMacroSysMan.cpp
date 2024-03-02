#include "pch.h"

typedef VOID(__cdecl *_CMacroSysMan__CreateInstance_t)();
_CMacroSysMan__CreateInstance_t _CMacroSysMan__CreateInstance = reinterpret_cast<_CMacroSysMan__CreateInstance_t>(0x00ADCE13);

void CMacroSysMan::CreateInstance() {
    _CMacroSysMan__CreateInstance();
}