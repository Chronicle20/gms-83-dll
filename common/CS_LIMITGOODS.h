#pragma once

// CS_LIMITGOODS: cash-shop limited-goods record. Real layout unknown.
// Used as `ZArray<CS_LIMITGOODS> m_aLimitGoods` in CWvsContext.
// ZArray<T> may need T complete depending on its inline methods; provide an
// opaque stub to be safe. Size is layout-irrelevant to CWvsContext.
// TODO: replace with real layout once available.
struct CS_LIMITGOODS {
    unsigned char _opaque[1];
};
