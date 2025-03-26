#include "pch.h"

CSystemInfo::CSystemInfo() {
    reinterpret_cast<void (__fastcall *)(CSystemInfo *, void *)>(
            C_SYSTEM_INFO)(this, nullptr);
}

unsigned char CSystemInfo::Init() {
    return reinterpret_cast<unsigned char (__fastcall *)(CSystemInfo *, void *)>(
            C_SYSTEM_INFO_INIT)(this, nullptr);
}

int CSystemInfo::GetGameRoomClient() {
    return reinterpret_cast<int (__fastcall *)(CSystemInfo *, void *)>(
            C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT)(this, nullptr);
}

unsigned __int8 *CSystemInfo::GetMachineId() {
    return reinterpret_cast<unsigned char *(__fastcall *)(CSystemInfo *, void *)>(
            C_SYSTEM_INFO_GET_MACHINE_ID)(this, nullptr);
}