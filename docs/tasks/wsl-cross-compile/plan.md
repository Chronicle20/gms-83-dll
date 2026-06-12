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

- [ ] **Step 2: Install LLVM, LLD, and ninja via Homebrew**

Homebrew is already on this machine (`/home/linuxbrew/.linuxbrew`), so no sudo is
needed. **NOTE (correction from bring-up):** Homebrew's `llvm` formula ships
`clang-cl`, `llvm-lib`, `llvm-rc`, and `lldb` (debugger) but **not** `lld` (the
linker). `lld-link` comes from the *separate* `lld` formula, and it installs into
a **different keg** (`$(brew --prefix lld)/bin`, not the llvm bin). Both are needed.

Run:
```bash
brew install llvm lld ninja
```
Expected: completes (llvm is ~1.5 GB; lld ~8 MB; may take several minutes).

- [ ] **Step 3: Verify the tools resolve across both kegs**

Both `llvm` and `lld` are keg-only, so their tools are NOT symlinked into the
default Homebrew bin. clang-cl/llvm-lib/llvm-rc live under the llvm keg; lld-link
under the lld keg:
```bash
LLVM_BIN="$(brew --prefix llvm)/bin"
LLD_BIN="$(brew --prefix lld)/bin"
for t in clang-cl llvm-lib llvm-rc; do printf "%-12s" "$t:"; [[ -x "$LLVM_BIN/$t" ]] && echo "OK ($LLVM_BIN/$t)" || echo MISSING; done
printf "%-12s" "lld-link:"; [[ -x "$LLD_BIN/lld-link" ]] && echo "OK ($LLD_BIN/lld-link)" || echo MISSING
printf "%-12s" "ninja:"; ninja --version >/dev/null 2>&1 && echo OK || echo MISSING
```
Expected: every tool prints `OK`. Record both `LLVM_BIN` and `LLD_BIN` — the driver
script (Task 4) and toolchain file (Task 3) reference these tools by absolute path
so a non-interactive shell finds them.

- [ ] **Step 4: Capture both bin paths for later tasks**

Run:
```bash
echo "LLVM_BIN=$(brew --prefix llvm)/bin"
echo "LLD_BIN=$(brew --prefix lld)/bin"
```
Expected: prints e.g. `LLVM_BIN=/home/linuxbrew/.linuxbrew/opt/llvm/bin` and
`LLD_BIN=/home/linuxbrew/.linuxbrew/opt/lld/bin`. No commit (host setup only).

---

### Task 2: Install xwin and generate the MSVC/SDK splat

**Files:** none tracked (splat lives outside the git tree at `~/.xwin-splat`).

- [ ] **Step 1: Install the xwin binary**

**CORRECTION (from bring-up):** xwin has a Homebrew formula (simpler than the
release tarball), and the current version is **0.9.0**, not 0.6.6. Install via brew:
```bash
brew install xwin
xwin --version
```
Expected: prints `xwin 0.9.0`. In 0.9.0 the `--arch` filter is a **global** option
(before the subcommand): `xwin --arch x86 splat ...`.

- [ ] **Step 2: Download + splat the CRT and Windows SDK (Win32/x86 only)**

This accepts the Microsoft redistributable license and writes ~1 GB to `~/.xwin-splat`.

**CORRECTION (from bring-up):** xwin *moves* files from its cache to the output by
default. In this WSL `/tmp` (cache) and `/home` (output) are different filesystems,
so the move fails with `Invalid cross-device link (EXDEV)`. Use `--copy` to copy
instead of move. Keep the cache on `/tmp` to reuse any prior download.
```bash
xwin --accept-license --arch x86 --cache-dir /tmp/.xwin-cache \
     splat --output "$HOME/.xwin-splat" --copy
```
Expected: completes silently with a `crt/` and `sdk/` tree under `~/.xwin-splat`.
Symlinks are on by default — that is what fixes `#include <Windows.h>` casing.

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

**FINDING (from bring-up):** The real file is `WS2_32.Lib`; xwin created symlinks
`ws2_32.lib` and `WS2_32.lib` (636 case symlinks in that dir total), so the
`#pragma comment(lib, "WS2_32.lib")` spelling resolves. BUT `cmake/CommonLib.cmake`
references the mixed-case `Ws2_32.lib` (capital W, lowercase `s2_32`), and xwin did
**not** create that exact permutation. This is the one casing gap; the driver script
(Task 4) creates the `Ws2_32.lib` symlink idempotently. All other libs
(`winmm.lib`, `comsuppw.lib`, vendored `detours.lib`) resolve as-is.

---

### Task 3: Write the CMake toolchain file

**Files:**
- Create: `cmake/toolchains/clang-cl-win32.cmake`

- [ ] **Step 1: Write the toolchain file**

**CORRECTION (from bring-up):** the toolchain file resolves `lld-link` from a
SEPARATE keg (`LLD_BIN`) than clang-cl/llvm-lib/llvm-rc (`LLVM_BIN`), per the
Task 1 finding. The committed file is the source of truth; create
`cmake/toolchains/clang-cl-win32.cmake` matching the repository copy. Its shape:
`CMAKE_SYSTEM_NAME=Windows`, `_PROCESSOR=x86`; resolve the splat from
`-DXWIN_SPLAT`/env/`~/.xwin-splat`; resolve tools from `-DLLVM_BIN` and `-DLLD_BIN`
(env fallback, then bare names on PATH); set `CMAKE_{C,CXX}_COMPILER=clang-cl`,
`CMAKE_LINKER=lld-link`, `CMAKE_AR=llvm-lib`, `CMAKE_RC_COMPILER=llvm-rc`;
`CMAKE_{C,CXX}_COMPILER_TARGET=i686-pc-windows-msvc`;
`CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY` (compile-only probe);
`/imsvc` include flags for `crt/include` + `sdk/include/{ucrt,um,shared}`;
`-fms-compatibility -fms-extensions -fasm-blocks` in `CMAKE_{C,CXX}_FLAGS_INIT`;
`/machine:x86` + `/libpath:` for `crt/lib/x86` + `sdk/lib/{um,ucrt}/x86` in the
linker `*_INIT` flags; and `CMAKE_FIND_ROOT_PATH` pinned to the splat.

- [ ] **Step 2: Verify configure succeeds (the toolchain probe)**

Run (note BOTH `LLVM_BIN` and `LLD_BIN`):
```bash
LLVM_BIN="$(brew --prefix llvm)/bin" LLD_BIN="$(brew --prefix lld)/bin" \
cmake -G Ninja -B /tmp/wsl-cfg-probe \
  --toolchain "$PWD/cmake/toolchains/clang-cl-win32.cmake" \
  -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1
```
Expected: ends with `-- Configuring done` / `-- Generating done`, with
`-- The C compiler identification is Clang 22.1.7 with MSVC-like command-line`.
(Bring-up result: PASS.) If it instead fails at "The C compiler ... is not able to
compile a simple test program", read
`/tmp/wsl-cfg-probe/CMakeFiles/CMakeError.log` — usually a missing `/imsvc` path or
a tool not found. Fix the toolchain file and re-run until configure passes.

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
