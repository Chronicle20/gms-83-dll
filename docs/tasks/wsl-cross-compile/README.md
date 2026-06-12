# WSL cross-compile check loop

Compile + link the Win32 edit DLLs inside WSL with `clang-cl` + `lld-link`, to
catch compile/link errors without a Windows round-trip.

**This is a dev-only check, not the authoritative build.** CI on `windows-2025`
(`.github/workflows/_build.yml`) owns the shipping artifacts. A successful WSL
build means "it compiles and links"; it is *not* a guarantee of byte/ABI parity
with the MSVC build, and the produced DLLs are not intended to be injected into
the client. To actually run a DLL, build on Windows.

## One-time setup

1. **Toolchain** (Homebrew, no sudo). `lld` is a *separate* formula from `llvm`
   (Homebrew's `llvm` ships `lldb`, not `lld`), and the two install to different
   keg prefixes — both are needed:
   ```bash
   brew install llvm lld ninja
   ```
2. **xwin + the MSVC/SDK splat** (~1 GB). The flags matter:
   - `--include-atl` — `common/pch.h` pulls in `<atlstr.h>` (ATL `CString`).
   - `splat --include-debug-libs` — a Debug build links the debug CRT
     (`msvcrtd.lib`, `comsuppwd.lib`, …).
   - `--copy` — in WSL the cache (`/tmp`) and output (`$HOME`) are usually on
     different filesystems; the default *move* fails with a cross-device error.
   ```bash
   brew install xwin
   xwin --accept-license --arch x86 --include-atl --cache-dir /tmp/.xwin-cache \
        splat --output "$HOME/.xwin-splat" --copy --include-debug-libs
   ```

## Usage

```bash
# defaults to GMS 83 1, all edit DLLs + proxy
./scripts/wsl-build.sh

# specific region/version (three separate tokens)
./scripts/wsl-build.sh GMS 87 1

# single target (fast iteration)
./scripts/wsl-build.sh GMS 83 1 skip-logo
```

Build output lands in `build-wsl/` (gitignored). The script resolves the LLVM and
LLD kegs automatically and self-heals the one SDK lib case-alias xwin omits
(`Ws2_32.lib`).

> Pass region/major/minor as **three separate arguments**, not one quoted string.
> Under `zsh`, `cfg="GMS 87 1"; wsl-build.sh $cfg` does NOT word-split and passes
> the whole string as the region. Use `wsl-build.sh GMS 87 1` or a bash array.

## Overrides

- `XWIN_SPLAT` — splat location (default `~/.xwin-splat`).
- `LLVM_BIN`  — clang-cl/llvm-lib/llvm-rc dir (default `$(brew --prefix llvm)/bin`).
- `LLD_BIN`   — lld-link dir (default `$(brew --prefix lld)/bin`).

## Supported configs

GMS 83.1, 84.1, 87.1, 95.1, 111.1; JMS 185.1 — matching the CI matrix. All six
verified to compile + link (11 DLLs each: 10 edit DLLs + `proxy/ijl15.dll`).

## How it works (and the WSL-only adaptations)

`cmake/toolchains/clang-cl-win32.cmake` targets `i686-pc-windows-msvc` with the
xwin splat on the include/lib search paths. Three differences from a stock MSVC
build, all confined to the toolchain (no client-source changes):

- **Manifest tool** — Homebrew's LLVM has no `llvm-mt`, and CMake's
  `find_program(mt)` otherwise grabs the Linux tape tool `/usr/bin/mt`. The
  toolchain sets `CMAKE_MT=/bin/true` to neutralize the (irrelevant) manifest
  embed step.
- **MS inline asm** — `proxy/ijl15.cpp`'s `__asm jmp` thunks compile under
  `-fms-extensions` (no `-fasm-blocks`, which clang-cl rejects as unknown).
- **EH force-include** — MSVC's frontend force-injects
  `<ehdata_forceinclude.h>` (declaring `_CxxThrowException` / `_ThrowInfo`) into
  every C++ TU; clang-cl does not. `cmake/toolchains/wsl-eh-forceinclude.h`
  supplies just those two symbols via `/FI` (CXX-only) so
  `bypass/client_exception.cpp` compiles. CI never sees this shim.

See `docs/tasks/wsl-cross-compile/design.md` and `plan.md` for the full rationale.
