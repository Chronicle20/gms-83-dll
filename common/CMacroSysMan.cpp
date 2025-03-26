#include "pch.h"

void CMacroSysMan::CreateInstance() {
    reinterpret_cast<void (__fastcall *)()>(C_MACRO_SYS_MAN_CREATE_INSTANCE)();
}