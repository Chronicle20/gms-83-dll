#include "pch.h"
#include "memory_map.h"

CSystemInfo::CSystemInfo() {
    ((VOID(_fastcall * )(CSystemInfo * , PVOID))
    C_SYSTEM_INFO)(this, NULL);
}

unsigned char CSystemInfo::Init() {
    return ((BYTE(_fastcall * )(CSystemInfo * , PVOID))
    C_SYSTEM_INFO_INIT)(this, NULL);
}

int CSystemInfo::GetGameRoomClient() {
    return ((int(_fastcall * )(CSystemInfo * , PVOID))
    C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT)(this, NULL);
}

unsigned __int8 *CSystemInfo::GetMachineId() {
    return ((unsigned __int8* (_fastcall * )(CSystemInfo * , PVOID))
            C_SYSTEM_INFO_GET_MACHINE_ID)(this, NULL);
}