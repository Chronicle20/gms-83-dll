#pragma once

// GW_Memo: real size unknown (not yet extracted from any PDB).
// Used as `ZList<GW_Memo> m_lReceivedMemo` in CWvsContext, which instantiates
// ZRefCountedDummy<GW_Memo> requiring the type to be complete. The opaque
// payload is layout-irrelevant to CWvsContext (the ZList header is 20 bytes
// regardless of T), but DOES affect heap allocations of GW_Memo instances.
// TODO: replace with real layout once available.
struct GW_Memo {
    unsigned char _opaque[1];
};
