#pragma once

// CUIFindFriend: UI dialog. Real layout unknown.
// Used as `ZRef<CUIFindFriend> m_pUIFindFriend` in CWvsContext (gated >= 87).
// Confirmed present in v87 (CUIFindFriend class with 10+ methods including
// OnCreate, OnButtonClicked, ListSorting, SendMyInfoRequest, OnMyInfoResult).
// TODO: replace with real layout once available.
class CUIFindFriend {
    unsigned char _opaque[1];
};
