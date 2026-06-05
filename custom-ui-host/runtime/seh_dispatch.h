#pragma once
#include <Windows.h>

#include "logger.h"

namespace custom_ui_host {

// Runs fn() catching any C++ exception (e.g. a _com_error thrown by a game
// CreateCtrl when a WZ resource fails to resolve). Kept separate from
// SafeDispatch because MSVC forbids a C++ try/catch and an __try/__except in
// the same function.
template <class F> inline void RunCatchingCpp(const char* siteName, F&& fn) {
    try {
        fn();
    } catch (...) {
        Log("custom-ui-host: C++ exception in consumer callback at site=[%s]", siteName);
    }
}

// Wraps a callable so that neither a C++ exception nor an access violation can
// escape into game code. SafeDispatch is noexcept, so a C++ exception reaching
// this boundary would otherwise std::terminate the process: RunCatchingCpp
// catches C++ exceptions, and the __except filter catches access violations.
// Other structured exceptions propagate. The callable must not construct or
// destruct C++ objects with non-trivial dtors directly at the SEH boundary —
// that restriction is enforced by C++ language rules, not by code here.
//
// Usage:
//   SafeDispatch("CustomUI button click", [&]() {
//       consumer_fn(arg);
//   });
template <class F> inline void SafeDispatch(const char* siteName, F&& fn) noexcept {
    __try {
        RunCatchingCpp(siteName, fn);
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER
                                                                 : EXCEPTION_CONTINUE_SEARCH) {
        Log("custom-ui-host: AV in consumer callback at site=[%s]", siteName);
    }
}

} // namespace custom_ui_host
