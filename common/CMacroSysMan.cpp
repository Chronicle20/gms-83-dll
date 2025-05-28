#include "pch.h"

void CMacroSysMan::CreateInstance() {
    Log("CMacroSysMan::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_MACRO_SYS_MAN_CREATE_INSTANCE)();
}