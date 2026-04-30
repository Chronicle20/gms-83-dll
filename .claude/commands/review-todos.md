---
description: Scan the codebase for TODO/FIXME markers and unimplemented stubs; updates docs/TODO.md — dispatches the todo-scanner agent
---

Dispatch the `todo-scanner` agent.

The agent runs a full-repo scan, categorizes findings by edit/component and priority, and updates `docs/TODO.md`. After it completes, surface the summary it returns: total findings, top critical items, and edits with the most concentrated incomplete work.
