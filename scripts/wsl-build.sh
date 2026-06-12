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

# Resolve the two keg-only Homebrew prefixes. clang-cl/llvm-lib/llvm-rc live in
# the llvm keg; lld-link in the separate lld keg.
if [[ -z "${LLVM_BIN:-}" ]] && command -v brew >/dev/null 2>&1; then
    LLVM_BIN="$(brew --prefix llvm)/bin"
fi
if [[ -z "${LLD_BIN:-}" ]] && command -v brew >/dev/null 2>&1; then
    LLD_BIN="$(brew --prefix lld)/bin"
fi
export LLVM_BIN LLD_BIN

die() { echo "error: $*" >&2; echo "see docs/tasks/wsl-cross-compile/README.md" >&2; exit 1; }

# --- preflight: tools + splat ---
[[ -n "${LLVM_BIN:-}" && -x "${LLVM_BIN}/clang-cl" ]] || die "clang-cl not found under '${LLVM_BIN:-unset}' (Task 1: brew install llvm)"
[[ -n "${LLD_BIN:-}"  && -x "${LLD_BIN}/lld-link"  ]] || die "lld-link not found under '${LLD_BIN:-unset}' (Task 1: brew install lld)"
command -v ninja >/dev/null 2>&1 || die "ninja not found (Task 1: brew install ninja)"
[[ -d "${SPLAT}/crt/include" ]] || die "xwin splat missing at '${SPLAT}' (Task 2: xwin splat)"

# --- self-heal known SDK lib case gaps ---
# xwin symlinks most casings, but cmake/CommonLib.cmake references the exact
# mixed-case spelling 'Ws2_32.lib' which xwin does not emit. Create it (and any
# future known gaps) idempotently, pointing at the real on-disk file.
ensure_lib_symlink() {
    local dir="$1" want="$2" real="$3"
    [[ -e "${dir}/${want}" ]] && return 0
    [[ -e "${dir}/${real}" ]] || { echo "warn: ${dir}/${real} absent; cannot alias ${want}" >&2; return 0; }
    ln -sf "${real}" "${dir}/${want}"
    echo ">> aliased ${want} -> ${real} in ${dir}"
}
ensure_lib_symlink "${SPLAT}/sdk/lib/um/x86" "Ws2_32.lib" "WS2_32.Lib"

echo ">> ${REGION} v${MAJOR}.${MINOR}  splat=${SPLAT}"
echo ">> llvm=${LLVM_BIN}  lld=${LLD_BIN}"
cmake -G Ninja -B "${BUILD_DIR}" \
    --toolchain "${TOOLCHAIN}" \
    -DXWIN_SPLAT="${SPLAT}" \
    -DLLVM_BIN="${LLVM_BIN}" \
    -DLLD_BIN="${LLD_BIN}" \
    -DBUILD_REGION="${REGION}" \
    -DBUILD_MAJOR_VERSION="${MAJOR}" \
    -DBUILD_MINOR_VERSION="${MINOR}"

if [[ -n "${TARGET}" ]]; then
    cmake --build "${BUILD_DIR}" --target "${TARGET}"
else
    cmake --build "${BUILD_DIR}"
fi
echo ">> OK"
