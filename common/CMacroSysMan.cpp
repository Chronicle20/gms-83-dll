#include "pch.h"
#include "memory_map.h"

void CMacroSysMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    C_MACRO_SYS_MAN_CREATE_INSTANCE)();
}