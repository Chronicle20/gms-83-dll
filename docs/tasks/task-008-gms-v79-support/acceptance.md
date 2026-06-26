# GMS v79.1 — Acceptance / Live Smoke Test (user-run)

This is the final gate (PRD §10, FR-16, R12). The agent prepares everything below;
**the launch itself is user-run** on a real Windows host with a real GMS v79
client. A correct build with a failed/blocked smoke test is reported as such — not
as success ([[verification-before-completion]]).

## Prerequisites
- CI is green for **GMS 79.1 Debug + Release** (the PR build matrix). Build-
  correctness is asserted by CI, not by the Linux/WSL box. `scripts/wsl-build.sh`
  can pre-flight the v79 build locally before handing off
  ([[project_wsl_cross_compile]]).
- A real **GMS v79** client install (the same binary the IDB was taken from).
- Windows 10/11. If you hit a Themida `"Cannot find 'ijl15.dll'"` error, install
  the [VC++ Redistributable (x86)](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170)
  — that is an environment issue (R12), not a build-correctness failure.

## Step 1 — Get the GMS 79.1 artifacts
Either:
- **From CI:** download the `artifacts-GMS-79.1-Release` (and/or `-Debug`)
  artifact from the green PR run, **or**
- **Local build:** build the repo with a VS2022 x86 toolchain configured for v79:
  `cmake -B build -G "Visual Studio 17" -A Win32 -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=79 -DBUILD_MINOR_VERSION=1`
  then build, then `--target package_dlls`. Artifacts land in `build/artifacts/`.

The artifact set contains the proxy `ijl15.dll` plus the per-edit DLLs (+ their
`.ini`s).

## Step 2 — Deploy (per README "Usage")
1. In the **root MapleStory v79 directory**, back up the original:
   `ijl15.dll` → `ijl15.dll.bak`.
2. Copy the **proxy `ijl15.dll`** into the root MapleStory directory.
3. Create an **`edits`** folder in the root MapleStory directory.
4. Copy the edit DLLs + their `.ini` files you want to exercise into `edits/`.
   For this smoke test use at least:
   - `redirect` (its provided `.ini` points the client at `localhost`)
   - `no-patcher`
   - `no-ad-balloon`
   - `bypass`
   - (optional) `no-beginner-party-block`, `no-enter-mts-map-restriction`,
     `enable-minimize`

## Step 3 — Launch & observe
Launch the v79 client and confirm:
- [ ] Client starts with **no crash** and **no Themida fault** (distinguish a true
      crash from a VC++-redist/Themida environment issue per R12).
- [ ] The **patcher window is suppressed** (`no-patcher` active).
- [ ] Client reaches the **title / login screen** (the FR-16 bar).
- [ ] `redirect` points the client at `localhost` (login attempt targets the
      configured IP — confirm via a local listener / server if available).
- [ ] No `bypass`-related fault at startup.
- [ ] On client termination, **no ad balloon** (`no-ad-balloon` active).

## Step 4 — Record results
Capture the outcome (pass/fail per checkbox, plus any crash address / Themida
message) back into this file or the PR description. **Do not** mark the task
complete on a green build alone — the live smoke test to title/login is the
confirmed acceptance bar (FR-16).
</content>
