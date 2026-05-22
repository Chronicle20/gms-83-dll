#pragma once

// CNoticeQuestProgress: real layout unknown (UI dialog class).
// Used as `ZRef<CNoticeQuestProgress> m_pNoticeQuestProgress` in CWvsContext.
// Opaque size is layout-irrelevant to CWvsContext (ZRef = 8 bytes).
// TODO: replace with real layout once available.
class CNoticeQuestProgress {
    unsigned char _opaque[1];
};
