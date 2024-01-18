#pragma once

class INetMsgHandler {
public:
    virtual void OnPacket(int, CInPacket *) = 0;
};