#pragma once

#include "CInPacket.h"
#include "ZList.h"
#include "ZSocketBase.h"
#include "ZInetAddr.h"
#include "ZFatalSection.h"
#include "ZXString.h"
#include "ZRef.h"
#include "ZSocketBuffer.h"
#include "COutPacket.h"

struct __POSITION {
};

class CClientSocket {
public:

    struct CONNECTCONTEXT {
        ZList<ZInetAddr> lAddr;
        __POSITION *posList;
        int bLogin;
    };

    HWND__ *m_hWnd;
    ZSocketBase m_sock;
    CClientSocket::CONNECTCONTEXT m_ctxConnect;
    ZInetAddr m_addr;
    int m_tTimeout;
    ZList<ZRef<ZSocketBuffer>> m_lpRecvBuff;
    ZList<ZRef<ZSocketBuffer>> m_lpSendBuff;
    CInPacket m_packetRecv;
    ZFatalSection m_lockSend;
    unsigned int m_uSeqSnd;
    unsigned int m_uSeqRcv;
    ZXString<char> m_URLGuestIDRegistration;
    int m_bIsGuestID;

    virtual ~CClientSocket() = default;

    static CClientSocket *GetInstance();

    void SendPacket(COutPacket *oPacket);
};
