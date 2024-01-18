#pragma once

class INetMsgHandler {
public:
    virtual void OnPacket(INetMsgHandler *, int, CInPacket *) = 0;
};