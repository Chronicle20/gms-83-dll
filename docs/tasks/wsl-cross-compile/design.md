# WSL clang-cl Cross-Compile Check Loop — Design

**Date:** 2026-06-12
**Status:** Design (pending review)
**Author:** brainstormed with Claude

## Problem

The edit DLLs in this repo are **Win32, MSVC-ABI** shared libraries injected into
the MapleStory client. The authoritative build runs on GitHub Actions
(`windows-2025` + MSVC, see `.github/workflows/_build.yml`). Claude works in WSL,
which has no MSVC toolchain, so every code change Claude makes must be handed to
the user via a git branch and built on Windows before any compile error surfaces.
That round-trip is the friction this design removes.

## Goal

Let Claude **configure, compile, and link** the edit DLLs entirely inside WSL, to
catch compile-time and link-time errors before the branch handoff. A successful
`compile + link` is the bar.

### Non-goals

- **Not** byte-for-byte / ABI-identical output vs the VS build. CI on
  `windows-2025` remains the single authoritative artifact source. (Decision:
  "compile-check loop only".)
- **Not** replacing the local Windows build the user uses to *run* DLLs in the
  live client. Injecting and running still requires Windows + the game.
- **Not** changing CI. `.github/workflows/*` is untouched.

The git-branch handoff for *running* a DLL in the client remains. Only the
*compile/link feedback* moves into WSL.

## Approach

**`clang-cl` + `lld-link` targeting `i686-pc-windows-msvc`, with the MSVC CRT and
Windows SDK supplied by [`xwin`](https://github.com/Jake-Shadle/xwin); Ninja
generator.**

Why this and not the alternatives:

- **clang-cl over MinGW (`i686-w64-mingw32`)** — clang-cl targets the *MSVC ABI*
  (name mangling, exception model, struct layout, vtable shape). MinGW uses a
  different CRT and ABI; given this project's whole point is faithful client RE
  (RTTI exception dispatch, exact struct layouts), a MinGW build could compile
  yet diverge silently from what ships. clang-cl keeps the ABI honest.
- **clang-cl is a `cl.exe`-compatible driver** — CMake's MSVC detection and flag
  generation Just Work; `#pragma comment(lib, ...)`, SEH (`__try/__except`), and
  `/imsvc` are all supported.
- **`lld-link` consumes MSVC COFF** — it links the existing vendored
  `common/detours.lib` (confirmed `!<arch>` i386 COFF, machine `0x014c`) and the
  Win32 SDK import libs directly. No need to rebuild Detours.
- **Ninja generator** — avoids the `Visual Studio 18 2026` generator + CMake ≥4.2
  coupling that CI carries. Local cmake is 4.3.2; Ninja has no such constraint.
- **xwin** — downloads the MSVC CRT + Windows SDK (headers + Win32 libs) from
  Microsoft's published redistributable manifests into a local "splat" directory,
  and normalizes the header-case issues that otherwise break `#include <Windows.h>`
  on a case-sensitive filesystem. This is precisely the problem xwin exists to
  solve.

## Components

1. **Host toolchain (one-time install).**
   `clang` (provides `clang-cl` + `lld-link`), `ninja`, and `xwin`. `cmake`
   (4.3.2) is already present via Homebrew. Install path documented in the
   dev-doc; the install itself is a manual prerequisite step, not scripted into
   the build (it needs the user's package manager / sudo).

2. **xwin SDK splat.**
   `xwin splat --output <dir>` produces `crt/{include,lib}` and
   `sdk/{include,lib}` with the Win32 (`x86`) libraries. Stored **outside the git
   tree** (default `~/.xwin-splat`, overridable via env var) — it is ~1 GB and
   license-gated (xwin records EULA acceptance). Never committed.

3. **CMake toolchain file** — `cmake/toolchains/clang-cl-win32.cmake`:
   - `CMAKE_SYSTEM_NAME=Windows`, `CMAKE_SYSTEM_PROCESSOR=x86`
   - `CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER=clang-cl`, `CMAKE_LINKER=lld-link`
   - `--target=i686-pc-windows-msvc`
   - `/imsvc` include flags pointing at `crt/include` and
     `sdk/include/{um,shared,ucrt}` from the splat
   - linker `/LIBPATH:` entries for `crt/lib/x86` and `sdk/lib/{um,ucrt}/x86`
   - `-fasm-blocks` so the MS-style `__asm` thunks in `proxy/ijl15.cpp` parse
   - splat location read from an env var (e.g. `XWIN_SPLAT`) with a sensible
     default, so the file isn't machine-specific

4. **Driver script** — `scripts/wsl-build.sh`:
   - Usage: `scripts/wsl-build.sh [REGION] [MAJOR] [MINOR] [TARGET]`,
     defaulting to `GMS 83 1` and the all-DLLs default target.
   - Runs `cmake -G Ninja -B build-wsl --toolchain
     cmake/toolchains/clang-cl-win32.cmake -DBUILD_REGION=… -DBUILD_MAJOR_VERSION=…
     -DBUILD_MINOR_VERSION=…` then `cmake --build build-wsl`.
   - `build-wsl/` is gitignored (the existing `build` / `cmake-*` ignore rules
     cover a chosen dir name; confirm the name is ignored).
   - Fails loudly if `clang-cl`, `lld-link`, `ninja`, or the splat dir are absent,
     pointing at the dev-doc.

5. **Dev-doc** — `docs/tasks/wsl-cross-compile/README.md` (or a section in the
   existing docs): how to install the toolchain, run xwin once, and invoke the
   driver. States plainly that this is a dev-only check, not the authoritative
   build.

## Validation / success criteria

A success ladder, each step gating the next:

1. **MVP proof:** `scripts/wsl-build.sh GMS 83 1 skip-logo` configures and **links
   `skip-logo` to a `.dll`** in WSL with zero errors. (`skip-logo` chosen as the
   simplest edit DLL.)
2. **Full default target:** `scripts/wsl-build.sh GMS 83 1` builds every edit DLL
   + `proxy` for GMS 83.1.
3. **Stretch — matrix coverage:** the other five CI configs (GMS 84.1, 87.1, 95.1,
   111.1; JMS 185.1) configure + build. They differ only by the included
   memory-map `.cmake` and compile defs, so this should be near-free once (2)
   works.

Out of scope for validation: byte/ABI equivalence with VS output.

## Tracked risks (and fallbacks)

| Risk | Detail | Fallback |
|---|---|---|
| **Header case-sensitivity** | `#include <Windows.h>` etc. vs lowercase files on a case-sensitive FS | xwin normalizes casing during splat; this is its core purpose. If gaps remain, add targeted symlinks. |
| **`comsuppw.lib`** | MSVC VCTools lib for `_bstr_t`/`_com_ptr_t`/`comdef`, used widely in `common/*.h`. Must be present to link. | Confirm xwin's CRT splat includes it. If absent, source the lib or (worst case) the affected DLLs stay compile-only. |
| **`__asm` thunks ×6** | `proxy/ijl15.cpp` uses MS inline asm (`jmp dword ptr[mem]`) | `-fasm-blocks` should handle these trivial jumps. If clang's MS-asm parser balks, `proxy` is the one DLL that may stay compile-only; flag before any source edit. |
| **SEH `__try/__except`** | `common/memedit.cpp` | clang-cl supports x86 SEH; low risk, listed for completeness. |
| **PCH `REUSE_FROM`** | `add_edit_dll` reuses `common_lib`'s PCH | All TUs here are clang-cl, so the shared PCH is self-consistent. The VS-produced PCH is irrelevant (separate `build-wsl/` dir). |

If a risk turns into a wall, the degradation is graceful and pre-agreed:
**fall back to compile-only** (objects, no link) for the affected target and report
it, rather than silently dropping coverage.

## What this explicitly does not touch

- `.github/workflows/*` — CI stays on `windows-2025` + MSVC, authoritative.
- Source code — no changes anticipated. The sole candidate is the inline-asm in
  `proxy/ijl15.cpp`, and only if `-fasm-blocks` is insufficient; any such edit is
  flagged for approval first.
- The existing VS / CLion build the user runs — left as-is.
