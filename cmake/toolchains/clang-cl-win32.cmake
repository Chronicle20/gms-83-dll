# Cross-compile the Win32 MSVC-ABI edit DLLs in WSL with clang-cl + lld-link,
# using an xwin-produced splat of the MSVC CRT + Windows SDK. Dev-only check
# loop; CI on windows-2025 remains authoritative. See
# docs/tasks/wsl-cross-compile/design.md.
#
# Tool locations come from two keg-only Homebrew formulae and therefore live in
# DIFFERENT directories:
#   LLVM_BIN -> clang-cl, llvm-lib, llvm-rc   ($(brew --prefix llvm)/bin)
#   LLD_BIN  -> lld-link                       ($(brew --prefix lld)/bin)
# scripts/wsl-build.sh resolves both and passes them as -D. When invoked
# directly without those vars, we fall back to bare names on PATH.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

# --- locate the xwin splat (override with -DXWIN_SPLAT=... or env XWIN_SPLAT) ---
if(NOT XWIN_SPLAT)
    if(DEFINED ENV{XWIN_SPLAT})
        set(XWIN_SPLAT "$ENV{XWIN_SPLAT}")
    else()
        set(XWIN_SPLAT "$ENV{HOME}/.xwin-splat")
    endif()
endif()
if(NOT EXISTS "${XWIN_SPLAT}/crt/include")
    message(FATAL_ERROR
        "xwin splat not found at '${XWIN_SPLAT}'. "
        "Run scripts/wsl-build.sh (it checks) or see "
        "docs/tasks/wsl-cross-compile/README.md.")
endif()

# --- locate the LLVM + LLD tools (override with -DLLVM_BIN/-DLLD_BIN or env) ---
if(NOT LLVM_BIN AND DEFINED ENV{LLVM_BIN})
    set(LLVM_BIN "$ENV{LLVM_BIN}")
endif()
if(NOT LLD_BIN AND DEFINED ENV{LLD_BIN})
    set(LLD_BIN "$ENV{LLD_BIN}")
endif()

if(LLVM_BIN)
    set(_clang_cl  "${LLVM_BIN}/clang-cl")
    set(_llvm_lib  "${LLVM_BIN}/llvm-lib")
    set(_llvm_rc   "${LLVM_BIN}/llvm-rc")
else()
    set(_clang_cl  clang-cl)
    set(_llvm_lib  llvm-lib)
    set(_llvm_rc   llvm-rc)
endif()
if(LLD_BIN)
    set(_lld_link "${LLD_BIN}/lld-link")
else()
    set(_lld_link lld-link)
endif()

set(CMAKE_C_COMPILER   "${_clang_cl}")
set(CMAKE_CXX_COMPILER "${_clang_cl}")
set(CMAKE_LINKER       "${_lld_link}")
set(CMAKE_AR           "${_llvm_lib}")
set(CMAKE_RC_COMPILER  "${_llvm_rc}")

# Neutralize the manifest tool. Homebrew's LLVM ships no llvm-mt, and CMake's
# find_program(mt) otherwise picks up the Linux magnetic-tape tool /usr/bin/mt,
# which breaks the post-link step. Manifest embedding is irrelevant to a
# compile/link check, so point CMAKE_MT at /bin/true: it is a real (found,
# non-empty) path so find_program won't override it, and it exits 0 -- which
# CMake's vs_link wrapper reads as "manifest not updated", keeping the linked
# DLL without an embed step. (An empty value does NOT work: find_program
# re-searches and re-finds /usr/bin/mt.)
set(CMAKE_MT "/bin/true" CACHE FILEPATH "manifest tool neutralized for the WSL check loop" FORCE)

# 32-bit MSVC target triple for both languages.
set(CMAKE_C_COMPILER_TARGET   i686-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET i686-pc-windows-msvc)

# Compile-only the toolchain probe so configure doesn't depend on a linkable
# entry point. Real link validation happens when we build the DLLs.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MSVC-system include dirs from the splat. /imsvc keeps them on the system path
# (no -Wnonportable-include-path noise from the SDK's own headers).
set(_xwin_incs
    "/imsvc${XWIN_SPLAT}/crt/include"
    "/imsvc${XWIN_SPLAT}/sdk/include/ucrt"
    "/imsvc${XWIN_SPLAT}/sdk/include/um"
    "/imsvc${XWIN_SPLAT}/sdk/include/shared")
string(JOIN " " _xwin_incflags ${_xwin_incs})

# -fms-extensions already enables the MS-style __asm jmp thunks in
# proxy/ijl15.cpp (verified: clang-cl compiles `__asm jmp dword ptr[p]` under
# -fms-extensions with no extra flag; -fasm-blocks is an unknown arg in clang-cl).
set(_common_flags "-fms-compatibility -fms-extensions ${_xwin_incflags}")
set(CMAKE_C_FLAGS_INIT   "${_common_flags}")
set(CMAKE_CXX_FLAGS_INIT "${_common_flags}")

# Library search paths for lld-link.
set(_xwin_libs
    "/libpath:${XWIN_SPLAT}/crt/lib/x86"
    "/libpath:${XWIN_SPLAT}/sdk/lib/um/x86"
    "/libpath:${XWIN_SPLAT}/sdk/lib/ucrt/x86")
string(JOIN " " _xwin_libflags ${_xwin_libs})
set(_link_init "/machine:x86 ${_xwin_libflags}")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${_link_init}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_link_init}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_link_init}")

# Only search the splat for headers/libs; never the (empty) host Windows roots.
set(CMAKE_FIND_ROOT_PATH "${XWIN_SPLAT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
