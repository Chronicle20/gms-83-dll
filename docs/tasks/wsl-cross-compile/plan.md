# WSL clang-cl Cross-Compile Check Loop — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let Claude configure, compile, and link the Win32 MSVC-ABI edit DLLs entirely inside WSL (via `clang-cl` + `lld-link` with MSVC headers/libs from `xwin`), so compile/link errors surface before the git-branch handoff.

**Architecture:** A CMake toolchain file points `clang-cl`/`lld-link` at an `xwin`-produced splat of the MSVC CRT + Windows SDK, targeting `i686-pc-windows-msvc`. A driver script wraps `cmake -G Ninja` + build for a given region/version. CI on `windows-2025` stays the authoritative artifact source; this is a dev-only check loop. The existing vendored `common/detours.lib` (i386 COFF) links directly.

**Tech Stack:** clang-cl, lld-link, llvm-lib, ninja, xwin, CMake 4.3.2, bash.

**Design reference:** `docs/tasks/wsl-cross-compile/design.md`

**Note on test style:** This is toolchain bring-up, not application code. "Tests" are
build/inspection commands with concrete expected output. The discipline is the same:
verify each step before moving on, commit frequently. Task 3 (the toolchain file) is the
empirical bring-up point — if any single task needs a debug iteration, it is that one;
its verification command is written to reveal the failure precisely.

---

### Task 1: Install host toolchain (clang-cl, lld-link, ninja)

**Files:** none (host environment only).

- [ ] **Step 1: Check what's already present**

Run:
```bash
for t in clang-cl lld-link llvm-lib ninja; do printf "%-12s" "$t:"; command -v $t || echo MISSING; done
```
Expected: likely all `MISSING` on a fresh WSL (only `cmake` is preinstalled via Homebrew).

- [ ] **Step 2: Install LLVM (provides clang-cl, lld-link, llvm-lib, llvm-rc) and ninja via Homebrew**

Homebrew is already on this machine (`/home/linuxbrew/.linuxbrew`), so no sudo is needed.

Run:
```bash
brew install llvm ninja
```
Expected: completes (llvm is ~1.5 GB; may take several minutes).

- [ ] **Step 3: Put the LLVM bin dir on PATH and verify the four tools resolve**

`brew install llvm` is keg-only, so its tools are NOT symlinked into the default
Homebrew bin. Resolve the prefix and verify:
```bash
LLVM_BIN="$(brew --prefix llvm)/bin"
for t in clang-cl lld-link llvm-lib llvm-rc ninja; do printf "%-12s" "$t:"; "$LLVM_BIN/$t" --version >/dev/null 2>&1 && echo "OK ($LLVM_BIN/$t)" || command -v "$t" || echo MISSING; done
```
Expected: `clang-cl`, `lld-link`, `llvm-lib`, `llvm-rc` resolve under `$(brew --prefix llvm)/bin`; `ninja` resolves on PATH. Record `LLVM_BIN` — the driver script (Task 4) and toolchain file (Task 3) reference these tools by absolute path so a non-interactive shell finds them.

- [ ] **Step 4: Capture the LLVM bin path for later tasks**

Run:
```bash
echo "LLVM_BIN=$(brew --prefix llvm)/bin"
```
Expected: prints e.g. `LLVM_BIN=/home/linuxbrew/.linuxbrew/opt/llvm/bin`. No commit (host setup only).

---

### Task 2: Install xwin and generate the MSVC/SDK splat

**Files:** none tracked (splat lives outside the git tree at `~/.xwin-splat`).

- [ ] **Step 1: Install the xwin binary**

Prefer a prebuilt release binary (no Rust toolchain needed). Run:
```bash
XWIN_VER=0.6.6
cd /tmp
curl -fsSL "https://github.com/Jake-Shadle/xwin/releases/download/${XWIN_VER}/xwin-${XWIN_VER}-x86_64-unknown-linux-musl.tar.gz" -o xwin.tgz
tar -xzf xwin.tgz
install -m755 "xwin-${XWIN_VER}-x86_64-unknown-linux-musl/xwin" "$(brew --prefix)/bin/xwin"
xwin --version
```
Expected: prints `xwin 0.6.6` (or the pinned version). If the download 404s, fall back to `cargo install xwin` (requires `brew install rust` first).

- [ ] **Step 2: Download + splat the CRT and Windows SDK (Win32/x86 only)**

This accepts the Microsoft redistributable license and writes ~1 GB to `~/.xwin-splat`.
Run:
```bash
xwin --accept-license --arch x86 splat --output "$HOME/.xwin-splat"
```
Expected: completes with a `crt/` and `sdk/` tree under `~/.xwin-splat`. (Downloads are cached under `./.xwin-cache`; delete `/tmp/.xwin-cache` afterward if created.)

- [ ] **Step 3: Verify the splat contains the headers and libs this project needs**

Run:
```bash
S="$HOME/.xwin-splat"
ls "$S/crt/include/vcruntime.h" \
   "$S/sdk/include/um/Windows.h" \
   "$S/crt/lib/x86/libcmt.lib" \
   "$S/crt/lib/x86/comsuppw.lib" \
   "$S/sdk/lib/um/x86/Ws2_32.lib" \
   "$S/sdk/lib/um/x86/winmm.lib" \
   "$S/sdk/lib/ucrt/x86/ucrt.lib"
```
Expected: every path lists without error. **This step retires two tracked risks from the design at once:** `comsuppw.lib` presence and SDK header availability. If `comsuppw.lib` is missing, stop and report — it is required by the COM-using headers in `common/` and there is no clean substitute.

- [ ] **Step 4: Confirm case-variant resolution for bare-name libs**

The code references `Ws2_32.lib`/`WS2_32.lib` (mixed case) via both CMake and
`#pragma comment(lib, ...)`. On a case-sensitive FS the linker must still find them.
Run:
```bash
S="$HOME/.xwin-splat"
ls -la "$S/sdk/lib/um/x86/" | grep -i 'ws2_32\.lib'
```
Expected: a `ws2_32.lib` entry exists (xwin lowercases SDK lib names and/or creates case symlinks by default). Note the actual casing — if only one case exists and it differs from a `#pragma comment(lib, ...)` spelling, that is the case-sensitivity risk materializing; the fallback is a symlink added in Task 4's script. No commit (host setup only).

---

### Task 3: Write the CMake toolchain file

**Files:**
- Create: `cmake/toolchains/clang-cl-win32.cmake`

- [ ] **Step 1: Write the toolchain file**

Create `cmake/toolchains/clang-cl-win32.cmake` with exactly:
```cmake
# Cross-compile the Win32 MSVC-ABI edit DLLs in WSL with clang-cl + lld-link,
# using an xwin-produced splat of the MSVC CRT + Windows SDK. Dev-only check
# loop; CI on windows-2025 remains authoritative. See
# docs/tasks/wsl-cross-compile/design.md.

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
        "Run scripts/wsl-build.sh (it checks/installs) or see "
        "docs/tasks/wsl-cross-compile/README.md.")
endif()

# --- locate the LLVM tools (override with -DLLVM_BIN=... or env LLVM_BIN) ---
if(NOT LLVM_BIN AND DEFINED ENV{LLVM_BIN})
    set(LLVM_BIN "$ENV{LLVM_BIN}")
endif()
if(LLVM_BIN)
    set(_clang_cl  "${LLVM_BIN}/clang-cl")
    set(_lld_link  "${LLVM_BIN}/lld-link")
    set(_llvm_lib  "${LLVM_BIN}/llvm-lib")
    set(_llvm_rc   "${LLVM_BIN}/llvm-rc")
else()
    set(_clang_cl  clang-cl)
    set(_lld_link  lld-link)
    set(_llvm_lib  llvm-lib)
    set(_llvm_rc   llvm-rc)
endif()

set(CMAKE_C_COMPILER   "${_clang_cl}")
set(CMAKE_CXX_COMPILER "${_clang_cl}")
set(CMAKE_LINKER       "${_lld_link}")
set(CMAKE_AR           "${_llvm_lib}")
set(CMAKE_RC_COMPILER  "${_llvm_rc}")

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

# -fasm-blocks: parse the MS-style __asm jmp thunks in proxy/ijl15.cpp.
set(_common_flags "-fms-compatibility -fms-extensions -fasm-blocks ${_xwin_incflags}")
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
```

- [ ] **Step 2: Verify configure succeeds (the toolchain probe)**

Run:
```bash
LLVM_BIN="$(brew --prefix llvm)/bin" \
cmake -G Ninja -B /tmp/wsl-cfg-probe \
  --toolchain "$PWD/cmake/toolchains/clang-cl-win32.cmake" \
  -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1
```
Expected: ends with `-- Configuring done` / `-- Generating done`, with
`-- The C compiler identification is Clang` and `MSVC`-like behavior detected.
If it fails at "The C compiler ... is not able to compile a simple test program",
read `/tmp/wsl-cfg-probe/CMakeFiles/CMakeError.log` — that is the precise bring-up
signal (usually a missing `/imsvc` path or a tool not found). Fix the toolchain file
and re-run until configure passes.

- [ ] **Step 3: Clean up the probe dir**

Run:
```bash
rm -rf /tmp/wsl-cfg-probe
```
Expected: no output.

- [ ] **Step 4: Commit**

```bash
git add cmake/toolchains/clang-cl-win32.cmake
git commit -m "build(wsl): clang-cl + xwin cross-compile toolchain file"
```

---

### Task 4: Write the driver script

**Files:**
- Create: `scripts/wsl-build.sh`
- Modify: `.gitignore` (add `build-wsl/`)

- [ ] **Step 1: Add the build dir to .gitignore**

The existing `build` and `cmake-*` patterns do NOT match `build-wsl`. Append a line
to `.gitignore`:
```
build-wsl
```

- [ ] **Step 2: Write the driver script**

Create `scripts/wsl-build.sh` with exactly:
```bash
#!/usr/bin/env bash
# Dev-only WSL cross-compile check loop. Configures + builds the Win32 edit
# DLLs with clang-cl + lld-link against an xwin splat. NOT the authoritative
# build -- CI on windows-2025 owns that. See docs/tasks/wsl-cross-compile/.
#
# Usage: scripts/wsl-build.sh [REGION] [MAJOR] [MINOR] [TARGET]
#   defaults: GMS 83 1  (all edit DLLs + proxy)
set -euo pipefail

REGION="${1:-GMS}"
MAJOR="${2:-83}"
MINOR="${3:-1}"
TARGET="${4:-}"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build-wsl"
TOOLCHAIN="${REPO_ROOT}/cmake/toolchains/clang-cl-win32.cmake"
SPLAT="${XWIN_SPLAT:-$HOME/.xwin-splat}"

# Resolve LLVM bin (keg-only Homebrew llvm is not on the default PATH).
if [[ -z "${LLVM_BIN:-}" ]]; then
    if command -v brew >/dev/null 2>&1; then
        LLVM_BIN="$(brew --prefix llvm)/bin"
    fi
fi
export LLVM_BIN

die() { echo "error: $*" >&2; echo "see docs/tasks/wsl-cross-compile/README.md" >&2; exit 1; }

# Preflight: tools + splat.
[[ -n "${LLVM_BIN:-}" && -x "${LLVM_BIN}/clang-cl" ]] || die "clang-cl not found under '${LLVM_BIN:-unset}' (Task 1: brew install llvm)"
command -v ninja >/dev/null 2>&1 || die "ninja not found (Task 1: brew install ninja)"
[[ -d "${SPLAT}/crt/include" ]] || die "xwin splat missing at '${SPLAT}' (Task 2: xwin splat)"

echo ">> ${REGION} v${MAJOR}.${MINOR}  splat=${SPLAT}  llvm=${LLVM_BIN}"
cmake -G Ninja -B "${BUILD_DIR}" \
    --toolchain "${TOOLCHAIN}" \
    -DXWIN_SPLAT="${SPLAT}" \
    -DBUILD_REGION="${REGION}" \
    -DBUILD_MAJOR_VERSION="${MAJOR}" \
    -DBUILD_MINOR_VERSION="${MINOR}"

if [[ -n "${TARGET}" ]]; then
    cmake --build "${BUILD_DIR}" --target "${TARGET}"
else
    cmake --build "${BUILD_DIR}"
fi
echo ">> OK"
```

- [ ] **Step 3: Make it executable**

Run:
```bash
chmod +x scripts/wsl-build.sh
```
Expected: no output.

- [ ] **Step 4: MVP proof — link the simplest edit DLL**

Run:
```bash
./scripts/wsl-build.sh GMS 83 1 skip-logo
```
Expected: ends with `>> OK`, and the DLL exists:
```bash
ls -la build-wsl/skip-logo/*.dll
```
Expected: a `skip-logo-*.dll` is listed. This is success-ladder step 1 from the
design — a real compile **and link** in WSL. If link fails on a missing symbol,
the error names the lib/symbol; cross-check against `cmake/CommonLib.cmake`'s lib
list and the splat (`Task 2 Step 3`).

- [ ] **Step 5: Commit**

```bash
git add scripts/wsl-build.sh .gitignore
git commit -m "build(wsl): driver script for clang-cl cross-compile check loop"
```

---

### Task 5: Build the full default target (all edit DLLs + proxy) for GMS 83.1

**Files:** none (exercises the existing tree end-to-end).

This task surfaces the remaining design risks together: the `__asm` thunks
(`proxy/ijl15.cpp`, needs `-fasm-blocks`), `comsuppw.lib`, SEH (`memedit.cpp`),
and the vendored `common/detours.lib` link.

- [ ] **Step 1: Build everything for GMS 83.1**

Run:
```bash
./scripts/wsl-build.sh GMS 83 1
```
Expected: ends with `>> OK`.

- [ ] **Step 2: Verify every edit DLL + proxy produced a .dll**

Run:
```bash
find build-wsl -name '*.dll' | sort
```
Expected: a `.dll` for each of bypass, doom-fix, enable-minimize, no-patcher,
no-beginner-party-block, no-enter-mts-map-restriction, no-ad-balloon, redirect,
skip-logo, window-mode, and proxy (11 DLLs).

- [ ] **Step 3: If the `proxy` inline-asm thunks fail to compile**

`-fasm-blocks` (set in the toolchain file) should handle the six
`__asm jmp dword ptr[...]` thunks. If clang's MS-asm parser still rejects them,
do NOT silently drop proxy. Build everything except proxy to confirm the rest is
green, and report the proxy failure for a decision (per design: proxy may stay
compile-only, or get a flagged source tweak):
```bash
for t in bypass doom-fix enable-minimize no-patcher no-beginner-party-block \
         no-enter-mts-map-restriction no-ad-balloon redirect skip-logo window-mode; do
    ./scripts/wsl-build.sh GMS 83 1 "$t"
done
```
Expected: each `>> OK`. (Skip this step entirely if Step 1 already passed.)

- [ ] **Step 4: No commit**

This task changes no tracked files — it is a verification gate. Proceed only once
Step 1 (or Step 3's fallback) is green.

---

### Task 6: Stretch — verify the other five matrix configs configure + build

**Files:** none.

These differ from GMS 83.1 only by the included memory-map `.cmake` and compile
defs, so they should build with no further toolchain work.

- [ ] **Step 1: Build each remaining CI config**

Run:
```bash
set -e
./scripts/wsl-build.sh GMS 84 1
./scripts/wsl-build.sh GMS 87 1
./scripts/wsl-build.sh GMS 95 1
./scripts/wsl-build.sh GMS 111 1
./scripts/wsl-build.sh JMS 185 1
```
Expected: each run ends with `>> OK`. (Each reconfigures `build-wsl` for the new
region/version.) If a specific version fails to compile, that is a real
version-gated source issue worth reporting — not a toolchain problem — since the
toolchain is identical across configs.

- [ ] **Step 2: No commit**

Verification gate only.

---

### Task 7: Write the dev-doc

**Files:**
- Create: `docs/tasks/wsl-cross-compile/README.md`

- [ ] **Step 1: Write the README**

Create `docs/tasks/wsl-cross-compile/README.md` with exactly:
```markdown
# WSL cross-compile check loop

Compile + link the Win32 edit DLLs inside WSL with `clang-cl` + `lld-link`, to
catch compile/link errors without a Windows round-trip.

**This is a dev-only check, not the authoritative build.** CI on `windows-2025`
(`.github/workflows/_build.yml`) owns the shipping artifacts. A successful WSL
build means "it compiles and links"; it is *not* a guarantee of byte/ABI parity
with the MSVC build, and the produced DLLs are not intended to be injected into
the client. To actually run a DLL, build on Windows.

## One-time setup

1. Toolchain (Homebrew, no sudo):
   ```bash
   brew install llvm ninja
   ```
2. xwin + the MSVC/SDK splat (~1 GB, accepts the MS redistributable license):
   ```bash
   xwin --accept-license --arch x86 splat --output "$HOME/.xwin-splat"
   ```
   (Install the `xwin` binary first — see the project plan, Task 2.)

## Usage

```bash
# defaults to GMS 83 1, all edit DLLs + proxy
./scripts/wsl-build.sh

# specific region/version
./scripts/wsl-build.sh GMS 87 1

# single target (fast iteration)
./scripts/wsl-build.sh GMS 83 1 skip-logo
```

Build output lands in `build-wsl/` (gitignored).

## Overrides

- `XWIN_SPLAT` — splat location (default `~/.xwin-splat`).
- `LLVM_BIN`  — LLVM tools dir (default `$(brew --prefix llvm)/bin`).

## Supported configs

GMS 83.1, 84.1, 87.1, 95.1, 111.1; JMS 185.1 — matching the CI matrix.
```

- [ ] **Step 2: Commit**

```bash
git add docs/tasks/wsl-cross-compile/README.md
git commit -m "docs(wsl-cross-compile): dev-doc for the WSL check loop"
```

---

## Self-Review

**Spec coverage:**
- clang-cl + lld-link + i686 target → Task 3. ✓
- xwin splat (outside tree, env-overridable) → Task 2, Task 3 (`XWIN_SPLAT`). ✓
- Reuse vendored `common/detours.lib` → exercised in Task 5 (no new code; CommonLib.cmake already references it by absolute path). ✓
- Ninja generator → Task 3 verify + Task 4 script. ✓
- `-fasm-blocks` for proxy asm → Task 3 toolchain file; risk-handled in Task 5 Step 3. ✓
- comsuppw.lib risk → Task 2 Step 3 (explicit check). ✓
- Header case-sensitivity risk → Task 2 Step 4 (explicit check + fallback). ✓
- Driver script defaults GMS 83.1, all DLLs → Task 4. ✓
- Success ladder (skip-logo → full 83.1 → matrix) → Tasks 4/5/6. ✓
- Dev-doc, dev-only caveat → Task 7. ✓
- No CI changes, no source changes (asm flagged, not edited) → respected throughout. ✓

**Placeholder scan:** No TBD/TODO; every code/config step shows full content; verification commands have concrete expected output.

**Type/name consistency:** `XWIN_SPLAT`, `LLVM_BIN`, `build-wsl`, `cmake/toolchains/clang-cl-win32.cmake`, and `scripts/wsl-build.sh` are spelled identically across the toolchain file, the driver script, and the docs.
