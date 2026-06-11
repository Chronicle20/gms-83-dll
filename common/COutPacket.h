#pragma once

class COutPacket {
public:
    bool m_bLoopback;
    ZArray<unsigned char> m_aSendBuff;
    unsigned int m_uOffset;
    bool m_bIsEncryptedByShanda;
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111
    // v111 appended a 4-byte field at +0x10: the ctor (sub_74C0C0) writes this[4]=0, while
    // this[1]=m_aSendBuff(+4) and this[2]=m_uOffset(+8) are unchanged from v83/v87. Pads 0x10 -> 0x14
    // so the real ctor's write lands in-bounds. (Verified in the v111 IDB: COutPacket ctor @0x74C290.)
    int dummy1;
#endif

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