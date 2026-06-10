#pragma once
#include <windows.h>

// Faithful client-RTTI exception raises. Each unwinds into the stock client
// WinMain handler via the client's own _ThrowInfo / COM-raise helpers. None return.

[[noreturn]] void RaiseClientException(int code); // maps m_hrZExceptionCode -> typed throw
[[noreturn]] void RaiseDisconnect(int code);      // 0x21000000-range (server migrate)
[[noreturn]] void RaiseTerminate(int code);       // 0x22000000-range
[[noreturn]] void RaisePatch();                   // ==0x20000000
[[noreturn]] void RaiseZException(int code);      // default
[[noreturn]] void RaiseComError(HRESULT hr);      // m_hrComErrorCode path
[[noreturn]] void RaiseComErrorEx(HRESULT hr);    // FAILED-render path
