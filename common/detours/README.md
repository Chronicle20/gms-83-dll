# Microsoft Detours — vendored binary

This directory exists to host provenance documentation for `common/detours.lib`
(checked in at `common/detours.lib`, one level up from this README). The binary
itself is intentionally not moved into this folder for this refactor.

## Upstream

- Project: [Microsoft Detours](https://github.com/microsoft/Detours)
- License: MIT — <https://github.com/microsoft/Detours/blob/main/LICENSE.md>

## Version

`unknown — pre-existing binary, version not recorded`

**TODO:** verify against the latest stable upstream release and record the
git tag or commit SHA here. Until that audit is done, treat this binary as a
black box of unknown vintage.

## Build

- Architecture: x86
- Toolchain: MSVC
- Kind: static library (`detours.lib`)

## Future

This binary blob will be replaced — by a source build of Detours, or by a
swap to MinHook — in a follow-up task. See
`docs/tasks/task-001-cmake-refactor/risks.md` (section "Compiler portability")
for the rationale and the suggested follow-up sequence.
