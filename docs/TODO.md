# Outstanding TODOs

## Build / CI

- Wire `BUILD_TESTS=ON` as a separate CI job (matrix entry or a follow-on
  workflow). Currently the host test target is developer-local only.
  Tracked: task-004-code-hygiene §4.9 open question.
- Have CI grep-assert no `return -1;` appears inside any `*_OR_RETURN`
  macro body in `common/hooker.h`. Prevents regressing the fix that made
  `INITMAPLEHOOK_OR_RETURN` return `FALSE` inside `BOOL Install*Hooks()`.

## Logger migration

- Migrate the 108 legacy `Log("...")` callsites in `bypass/`, `common/`,
  `redirect/`, `skip-logo/`, `proxy/` to the severity-gated `LOG_INFO` /
  `LOG_WARN` / `LOG_ERROR` macros added in task-004 §4.8. The new
  infrastructure is shipped but unused — risk of bit-rot.

## Bypass / socket

- `bypass/socket_hooks.cpp::read_packet_header` issues a single `recv`
  rather than looping until 2 bytes have accumulated. On a slow / poorly
  buffered link a short-read of 1 byte would corrupt `expectedLen` and
  cascade into a length-mismatched body read. Fix: delegate to
  `read_packet_body(pSock, out, 2, retries)` (one line). Plan-level
  design call from task-004 §4.2; flagged in post-merge review.

## Common

- `common/byte_ops::copy` currently has zero production callers (only
  the host tests exercise it). Either delete or add a real caller in a
  future task.

## Memedit

- `common/memedit.h:16` `#define x86NOP` is dead after task-004 §4.9 prep
  routed `MemEdit::PatchNop` through `ms::byte_ops::fill_nop`. Remove on
  the next memedit touch.

## Known engine bugs (not ours)

- Mob re-entry crash: `CMob::OnResolveMoveAction` dereferences
  `CMob::m_pvc` (offset 0x11C on v79) without a null check; when a mob
  re-enters the field before its vector controller is attached the
  client AVs. Reproduced on v79; static analysis shows v95 shares the
  bug. Temporary instrumentation + workaround (resolve against the
  first controller when `m_pvc` is null) was developed on task-008 and
  reverted before merge — see the six `debug(v79)` commits ending at
  `e1b5d00` for the full investigation. A proper fix would be a new
  standalone edit (null-guard hook), not part of a version port.
