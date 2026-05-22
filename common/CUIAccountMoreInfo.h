#pragma once

// CUIAccountMoreInfo: UI dialog. Real layout unknown.
// Used as `ZRef<CUIAccountMoreInfo> m_pUIAccountMoreInfo` in CWvsContext (gated >= 87).
// Confirmed present in v87 (?OnAccountMoreInfo@CWvsContext@@QAEXAAVCInPacket@@@Z exists).
// TODO: replace with real layout once available.
class CUIAccountMoreInfo {
    unsigned char _opaque[1];
};
