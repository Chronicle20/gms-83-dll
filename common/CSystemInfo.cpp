#include "pch.h"

// void __thiscall CSystemInfo::CSystemInfo(CSystemInfo *this)
CSystemInfo::CSystemInfo() {
    ((VOID(_fastcall * )(CSystemInfo * , PVOID))
    0x00A54B90)(this, NULL);
}


// _BYTE *__thiscall CSystemInfo::Init(CSystemInfo *this)
//typedef BYTE(__fastcall *_CSystemInfo__Init_t)(CSystemInfo *pThis, PVOID edx);
//_CSystemInfo__Init_t _CSystemInfo__Init = reinterpret_cast<_CSystemInfo__Init_t>(0x00A54BD0);
unsigned char CSystemInfo::Init() {
    return ((BYTE(_fastcall * )(CSystemInfo * , PVOID))
    0x00A54BD0)(this, NULL);
}


// int __thiscall CSystemInfo::GetGameRoomClient(CSystemInfo *this)
// typedef INT(__fastcall *_CSystemInfo__GetGameRoomClient_t)(CSystemInfo *pThis, PVOID edx);
// _CSystemInfo__GetGameRoomClient_t _CSystemInfo__GetGameRoomClient = reinterpret_cast<_CSystemInfo__GetGameRoomClient_t>(0x00A54FB0);
int CSystemInfo::GetGameRoomClient() {
    return ((int(_fastcall * )(CSystemInfo * , PVOID))
    0x00A54FB0)(this, NULL);
}

unsigned __int8 *CSystemInfo::GetMachineId() {
    return ((unsigned __int8* (_fastcall * )(CSystemInfo * , PVOID))
    0x00A54EB0)(this, NULL);
}