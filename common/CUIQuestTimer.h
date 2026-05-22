#pragma once

// CUIQuestTimer: real layout unknown (UI timer class).
// Used as `ZList<ZRef<CUIQuestTimer>> m_lpUIQuestTimer` in CWvsContext.
// ZRef<T> requires T complete. Opaque size is layout-irrelevant to CWvsContext.
// TODO: replace with real layout once available.
class CUIQuestTimer {
    unsigned char _opaque[1];
};
