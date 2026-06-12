// WSL cross-build ONLY -- force-included into every TU via /FI by
// cmake/toolchains/clang-cl-win32.cmake. Never referenced by the MSVC/CI build.
//
// Why this exists:
//   bypass/client_exception.cpp calls _CxxThrowException and casts client RTTI
//   tables to _ThrowInfo*, relying on those names being visible "through
//   <windows.h>" (see the comment in that file). On MSVC that holds because the
//   c1xx frontend preprocesses <ehdata_forceinclude.h> and force-injects it into
//   every C++ TU. clang-cl performs no such injection, and that header is not
//   directly includable (it carries MSVC pre-pass tokens like `prepifdef`).
//
//   So for the clang-cl cross-build we declare exactly the two symbols the
//   faithful-RTTI code needs, with the MSVC ABI signature copied verbatim from
//   ehdata_forceinclude.h (extern "C", __declspec(noreturn), __stdcall). The
//   client's per-version _ThrowInfo tables are referenced only as pointers, so an
//   incomplete type is sufficient and avoids duplicating MSVC's struct layout.
//
//   This file is invisible to MSVC/CI, so it cannot trigger the C2733 (mismatched
//   prototype) / C2371 (redefinition) clashes that a local declaration inside the
//   .cpp would cause there.
#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#error "wsl-eh-forceinclude.h is for the clang-cl cross-build only; never compile it with MSVC."
#endif

// C++-only (extern "C" is a syntax error in C). The toolchain force-includes
// this for CXX TUs only; the guard is belt-and-suspenders.
#ifdef __cplusplus
struct _ThrowInfo;

extern "C" __declspec(noreturn) void __stdcall _CxxThrowException(void* pExceptionObject, _ThrowInfo* pThrowInfo);
#endif
