# GMS v84.1 — Acceptance / Live Smoke Test (user-run)

This is the final gate (PRD §10, FR-16, R8). The agent prepared everything below;
**the launch itself is user-run** on a real Windows host with a real GMS v84 client.
A correct build with a failed/blocked smoke test is reported as such — not as success.

## Prerequisites
- CI is green for **GMS 84.1 Debug + Release** (PR #58 build matrix). Build-correctness
  is asserted by CI, not by this Linux/WSL box.
- A real **GMS v84** client install (the same binary the IDB was taken from:
  `GMS_v84.1_U_DEVM`-family client).
- Windows 10/11. If you hit a Themida `"Cannot find 'ijl15.dll'"` error, install the
  [VC++ Redistributable (x86)](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170)
  — that is an environment issue (R8), not a build-correctness failure.

## Step 1 — Get the GMS 84.1 artifacts
Either:
- **From CI:** download the `artifacts-GMS-84.1-Release` (and/or `-Debug`) artifact from the
  green PR #58 run, **or**
- **Local build:** build the repo with a VS2022 x86 toolchain configured for v84:
  `cmake -B build -G "Visual Studio 17" -A Win32 -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=84 -DBUILD_MINOR_VERSION=1`
  then build, then `--target package_dlls`. Artifacts land in `build/artifacts/`.

The artifact set contains the proxy `ijl15.dll` plus the per-edit DLLs (+ their `.ini`s).

## Step 2 — Deploy (per README "Usage")
1. In the **root MapleStory v84 directory**, back up the original: `ijl15.dll` → `ijl15.dll.bak`.
2. Copy the **proxy `ijl15.dll`** into the root MapleStory directory.
3. Create an **`edits`** folder in the root MapleStory directory.
4. Copy the edit DLLs + their `.ini` files you want to exercise into `edits/`. For this
   smoke test use at least:
   - `redirect` (its provided `.ini` points the client at `localhost`)
   - `no-patcher`
   - `no-ad-balloon`
   - `bypass`
   - (optional) `no-beginner-party-block`, `no-enter-mts-map-restriction`, `enable-minimize`

## Step 3 — Launch & observe
Launch the v84 client and confirm:
- [ ] Client starts with **no crash** and **no Themida fault** (distinguish a true crash
      from the VC++ redist / OS environment issue above).
- [ ] `no-patcher`: the MapleStory **patcher window does not appear** on launch.
- [ ] Reaches the **logo → title/login** screen cleanly (exercises CLogo/CLogin/CStage —
      the structs whose v84 gates were audited).
- [ ] `redirect`: the client attempts its socket connection to **localhost** (verify with a
      local listener / packet capture, or that it tries 127.0.0.1 instead of the live IP).
- [ ] `no-ad-balloon`: on client termination, the **ad balloon/window does not appear**.
- [ ] (if a local v84 server is available) log in far enough to confirm the
      **client-key handshake** and stat decode don't desync (exercises CWvsContext
      `m_aClientKey[8]` + the SecondaryStat/CUIToolTip layout fixes from Task 16).
- [ ] (optional) `no-beginner-party-block` / `no-enter-mts-map-restriction` behave as described.

## Step 4 — Record the result
Paste the **verbatim outcome** below (what launched, what each edit did, any crash text /
Themida message verbatim). The task is **not done** until this is filled in.

```
DATE:
CLIENT BUILD:
ARTIFACT SOURCE (CI run # / local):
RESULT (verbatim):


```

**Build-correctness vs environment (R8):** if the only failures are Themida / VC++ redist /
OS-permission issues, note them as environment — the build itself may still be correct. A
genuine crash in a targeted edit's code path IS a build-correctness failure and must be fixed
before merge.
