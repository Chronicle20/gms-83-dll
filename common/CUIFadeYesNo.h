#pragma once

// CUIFadeYesNo: real layout unknown (UI dialog class).
// Used as `ZArray<ZRef<CUIFadeYesNo>> m_apFadeWnd` in CWvsContext.
// ZRef<T> requires T complete due to its ZRefCountedAccessor<ZRefCountedDummy<T>> base.
// Opaque size is layout-irrelevant to CWvsContext (ZRef = 8 bytes, ZArray header = 4 bytes).
// TODO: replace with real layout once available.
class CUIFadeYesNo {
    unsigned char _opaque[1];
};
