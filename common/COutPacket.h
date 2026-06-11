#pragma once

class COutPacket {
public:
    bool m_bLoopback;
    ZArray<unsigned char> m_aSendBuff;
    unsigned int m_uOffset;
    bool m_bIsEncryptedByShanda;

    explicit COutPacket(INT nType);
    ~COutPacket();

    VOID Encode1(unsigned __int8 n);
    VOID Encode2(unsigned short n);
    VOID Encode4(UINT n);
    VOID EncodeStr(ZXString<char> s);
    VOID EncodeBuffer(const void *p, unsigned int uSize);

    VOID MakeBufferList(ZList<ZRef<ZSocketBuffer>>* l, unsigned short uSeqBase, unsigned int* puSeqKey, int bEnc,
                        unsigned int dwKey) const;
};

// COutPacket: stack-instantiated by value; the real ctor + Encode* write into it -> our struct
// (0x10: m_bLoopback / m_aSendBuff / m_uOffset / m_bIsEncryptedByShanda) must be >= the real
// size. Real (size sweep): v83/v84/v87 = 0x10, v111 = 0x14, JMS = 0x0C (v95 TBD).
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111
static_assert(sizeof(COutPacket) >= 0x14, "COutPacket too small for GMS v111 (need >= 0x14)");
#elif defined(REGION_GMS)
static_assert(sizeof(COutPacket) >= 0x10, "COutPacket too small (need >= 0x10)");
#elif defined(REGION_JMS)
static_assert(sizeof(COutPacket) >= 0x0C, "COutPacket too small for JMS (need >= 0x0C)");
#endif