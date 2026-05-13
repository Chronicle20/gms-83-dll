#pragma once

// RAII guard matching the game's ZSynchronizedHelper<L> template. The ctor
// spin-acquires the lock; the dtor releases it. The class is layout-compatible
// with the game's struct (single m_pLock pointer). Specializations delegate to
// the game's compiled ctor/dtor at their version-specific addresses.
template <typename L> class ZSynchronizedHelper {
  public:
    L* m_pLock;

    explicit ZSynchronizedHelper(L* lock);
    ~ZSynchronizedHelper();

    ZSynchronizedHelper(const ZSynchronizedHelper&) = delete;
    ZSynchronizedHelper& operator=(const ZSynchronizedHelper&) = delete;
};

template <> ZSynchronizedHelper<ZFatalSection>::ZSynchronizedHelper(ZFatalSection* lock);

template <> ZSynchronizedHelper<ZFatalSection>::~ZSynchronizedHelper();
