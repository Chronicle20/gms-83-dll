#pragma once

struct __POSITION {
};

class CClientSocket {
public:

    struct CONNECTCONTEXT {
        ZList<ZInetAddr> lAddr;
        __POSITION *posList;
        int bLogin;
    };

#if defined(REGION_JMS)
    char dummy1;
#endif
    HWND__ *m_hWnd;
    ZSocketBase m_sock;
    CClientSocket::CONNECTCONTEXT m_ctxConnect;
    ZInetAddr m_addr;
#if (defined(REGION_GMS) && MAJOR_VERSION >= 111)
    int dummy1;
#endif
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

    static void CreateInstance();

    void SendPacket(COutPacket *oPacket);

    void ManipulatePacket();

    void Close();

    void ClearSendReceiveCtx();

    void ConnectLogin();
};
