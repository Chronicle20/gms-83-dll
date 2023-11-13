#pragma once

// Exclude rarely-used stuff from Windows headers
// Important to define this before Windows.h is included in a project because of linker issues with the WinSock2 lib
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <functional>
#include "hooker.h"
#include "logger.h"
#include "winhooks.h"
#include "winhook_types.h"
#include "FakeModule.h"

#define MAPLE_MULTICLIENT FALSE

/// <summary>
/// 
/// </summary>
class Common {
private:
    struct Config {
    public:
        const char *MapleMutex = "WvsClientMtx";

        DWORD LocaleSpoofValue;
        DWORD SleepAfterUnpackDuration;

        Config() {
            LocaleSpoofValue = 0;
            SleepAfterUnpackDuration = 0;
        }
    };

private:
    static Common *_s_pInstance;
    static Common::Config *_s_pConfig;

public: // public because all the C-style hooks have to access these members
    BOOL m_bThemidaUnpacked;
    FakeModule *m_pFakeHsModule;

    /// <summary>
    /// Gets called when mutex hook is triggered.
    /// </summary>
    std::function<void()> m_PostMutexFunc;

private: // forcing the class to only have one instance, created through CreateInstance
    Common(BOOL bHookWinLibs, std::function<void()> pPostMutexFunc);

    Common() = delete;

    Common(const Common &) = delete;

    Common &operator=(const Common &) = delete;

public:
    ~Common();

    /// <summary>
    /// Function called from library hooks.
    /// Most of the time this should be triggered by the Mutex hook, however, in the case that
    /// the Mutex hook does not get triggered then this will be executed by CreateWindowExA
    /// for redundancy. The contents of this function will only be executed once, even if both
    /// Mutex and CreateWindow hooks are called properly.
    /// </summary>
    void OnThemidaUnpack();

public:
    static void CreateInstance(BOOL bHookWinLibs, std::function<void()> pMutexFunc) {
        if (_s_pInstance) return;

        _s_pInstance = new Common(bHookWinLibs, pMutexFunc);
    }

    static Common *GetInstance() {
        return _s_pInstance;
    }

    static Config *GetConfig() {
        if (!_s_pConfig) _s_pConfig = new Config();

        return _s_pConfig;
    }
};

