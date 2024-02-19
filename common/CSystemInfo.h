#pragma once

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
