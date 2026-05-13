#include "pch.h"

template <> ZSynchronizedHelper<ZFatalSection>::ZSynchronizedHelper(ZFatalSection* lock) {
    reinterpret_cast<void(__fastcall*)(ZSynchronizedHelper<ZFatalSection>*, void*, ZFatalSection*)>(
        Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR)(this, nullptr, lock);
}

template <> ZSynchronizedHelper<ZFatalSection>::~ZSynchronizedHelper() {
    reinterpret_cast<void(__fastcall*)(ZSynchronizedHelper<ZFatalSection>*, void*)>(
        Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR)(this, nullptr);
}
