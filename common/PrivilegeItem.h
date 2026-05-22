#pragma once

// PrivilegeItem: family-system privilege entry. Real layout unknown.
// Used as `ZArray<ZRef<PrivilegeItem>> m_apPrivilege` in CWvsContext.
// ZRef<T> requires T complete. Opaque size is layout-irrelevant to CWvsContext.
// TODO: replace with real layout once available.
struct PrivilegeItem {
    unsigned char _opaque[1];
};
