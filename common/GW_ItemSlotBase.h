
struct GW_ItemSlotBase : ZRefCounted {
    TSecType<long> nItemID;
    _LARGE_INTEGER liCashItemSN;
    _FILETIME dateExpire;
};
