#include "client_exception.h"
#include "memory_map.h"
#include "CWvsApp.h"

// MSVC CRT EH entry; we pass the CLIENT's _ThrowInfo so the client's
// __CxxFrameHandler (installed by WinMain) matches the catch by its own RTTI.
// The second parameter must be _ThrowInfo* to match MSVC's canonical extern "C"
// declaration (defined in <ehdata.h>); declaring it as void* is rejected with
// C2733 ("cannot overload a function with extern C linkage"). We only need the
// pointer type, so a forward declaration of the tag is sufficient.
struct _ThrowInfo;
extern "C" void __stdcall _CxxThrowException(void* pObject, _ThrowInfo* pThrowInfo);

namespace {
// Confirmed per-version (signature-catalog.md): every client builds CPatchException via a
// __thiscall ctor(buffer, version); the COM raiser is the 1-arg __stdcall _com_error raiser
// (?_com_issue_error@@YGXJ@Z / v84 sub_AABF64), reused for both COM paths.
using PatchCtorFn = void*(__thiscall*)(void*, int);  // ctor(buf, version) -> buf
using ComRaiseFn  = void(__stdcall*)(HRESULT);       // throws _com_error(hr); 1-arg __stdcall
} // namespace

[[noreturn]] void RaiseDisconnect(int code) {
    _CxxThrowException(&code, reinterpret_cast<_ThrowInfo*>(C_TI_DISCONNECT_EXCEPTION));
    __assume(0);
}

[[noreturn]] void RaiseTerminate(int code) {
    _CxxThrowException(&code, reinterpret_cast<_ThrowInfo*>(C_TI_TERMINATE_EXCEPTION));
    __assume(0);
}

[[noreturn]] void RaiseZException(int code) {
    _CxxThrowException(&code, reinterpret_cast<_ThrowInfo*>(C_TI_ZEXCEPTION));
    __assume(0);
}

[[noreturn]] void RaisePatch() {
    // All six clients build CPatchException via a __thiscall ctor(buffer, version) that
    // constructs into a caller buffer and returns it. _CxxThrowException copies the object
    // (size per the client _ThrowInfo) synchronously before any unwinding, so the stack
    // buffer is safe. 2048 >= any version's CPatchException (v84 = 1288 bytes).
    unsigned char buf[2048];
    auto ctor = reinterpret_cast<PatchCtorFn>(C_PATCH_EXCEPTION_BUILDER);
    void* obj = ctor(buf, CWvsApp::GetInstance()->m_nTargetVersion);
    _CxxThrowException(obj, reinterpret_cast<_ThrowInfo*>(C_TI_PATCH_EXCEPTION));
    __assume(0);
}

[[noreturn]] void RaiseClientException(int code) {
    if (code == 0x20000000) {
        RaisePatch();
    }
    if (code >= 0x21000000 && code <= 0x21000007) {
        RaiseDisconnect(code);
    }
    if (code >= 0x22000000 && code <= 0x2200000E) {
        RaiseTerminate(code);
    }
    RaiseZException(code);
}

// Both COM paths throw _com_error(hr) via the single 1-arg __stdcall raiser. Kept as two
// named entry points so the call sites read intentionally (m_hrComErrorCode vs FAILED-render).
[[noreturn]] void RaiseComError(HRESULT hr) { // m_hrComErrorCode path
    reinterpret_cast<ComRaiseFn>(C_COM_RAISE_ERROR_EX)(hr);
    __assume(0);
}

[[noreturn]] void RaiseComErrorEx(HRESULT hr) { // FAILED-render path
    reinterpret_cast<ComRaiseFn>(C_COM_RAISE_ERROR_EX)(hr);
    __assume(0);
}
