#pragma once

// CClock: UI clock widget. Real layout unknown.
// Used as `ZRef<CClock> m_pClock` in CWvsContext (gated >= 87).
// Confirmed present in v87 (CClock::CClock, CClock::SetTimer, CClock::Start
// exist as named functions). Layout-irrelevant to CWvsContext (ZRef = 8 bytes).
// TODO: replace with real layout once available.
class CClock {
    unsigned char _opaque[1];
};
