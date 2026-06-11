#pragma once

#include "asserts.h"

class CSystemInfo {
public:
    virtual ~CSystemInfo() = default;

    unsigned __int8 SupportId[16];
    unsigned __int8 MachineId[16];

    explicit CSystemInfo();
    unsigned char Init();
    int GetGameRoomClient();
    unsigned __int8 *GetMachineId();
};

// Stack-instantiated by value in CClientSocket::OnConnect; the real ctor/Init() write
// into it, so an undersized struct would smash the stack. vtable + SupportId[16] +
// MachineId[16]. Verified identical in v84 (Init ends at +0x24) and v87.
assert_size(sizeof(CSystemInfo), 0x24)
