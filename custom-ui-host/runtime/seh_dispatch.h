#pragma once
#include <Windows.h>

#include "logger.h"

namespace custom_ui_host {

// Wraps a callable in a __try/__except filtering on EXCEPTION_ACCESS_VIOLATION.
// Other exceptions propagate normally. The callable must not construct or
// destruct C++ objects with non-trivial dtors at the SEH boundary — that
// restriction is enforced by C++ language rules, not by code here.
//
// Usage:
//   SafeDispatch("CustomUI button click", [&]() {
//       consumer_fn(arg);
//   });
template <class F> inline void SafeDispatch(const char* siteName, F&& fn) noexcept {
    __try {
        fn();
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER
                                                                 : EXCEPTION_CONTINUE_SEARCH) {
        Log("custom-ui-host: AV in consumer callback at site=[%s]", siteName);
    }
}

} // namespace custom_ui_host
