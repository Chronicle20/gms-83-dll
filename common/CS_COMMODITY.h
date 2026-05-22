#pragma once

// CS_COMMODITY: cash-shop commodity record. Real layout unknown.
// Used as `ZArray<ZRef<CS_COMMODITY>>` in CWvsContext (m_aOriginalCommodity, m_aCommodity).
// ZRef<T> requires T complete. Opaque size is layout-irrelevant to CWvsContext.
// TODO: replace with real layout once available.
struct CS_COMMODITY {
    unsigned char _opaque[1];
};
