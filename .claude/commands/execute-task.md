---
description: Phase 4 — invoke superpowers:subagent-driven-development to implement a planned task, with subagent-per-task isolation
argument-hint: Task folder name under docs/tasks/ (e.g., "task-003-no-pet-loot")
---

You are starting Phase 4 of the four-phase development workflow. The task folder is: **$ARGUMENTS**

## Process

### Step 1 — Validate input

1. Resolve `docs/tasks/$ARGUMENTS/`. Confirm `plan.md` exists.
2. Confirm `context.md` exists alongside.
3. If either is missing, stop and tell the user to complete `/plan-task` first.

### Step 2 — Confirm execution mode

Ask the user once: subagent-driven (recommended) or inline?

- **Subagent-driven (default):** fresh subagent per task, two-stage review between tasks. Use `superpowers:subagent-driven-development`.
- **Inline:** batch execution in current session with checkpoints. Use `superpowers:executing-plans`.

If the user does not respond within the same message, default to subagent-driven.

### Step 3 — Recommend a worktree

If the current branch is `main` (or another protected branch), strongly recommend invoking `superpowers:using-git-worktrees` first to create an isolated workspace. Do NOT begin implementation on `main` without explicit user consent.

### Step 4 — Invoke the chosen execution skill

Use the Skill tool to invoke either `superpowers:subagent-driven-development` or `superpowers:executing-plans`. Pass:

- Plan path: `docs/tasks/$ARGUMENTS/plan.md`
- Context path: `docs/tasks/$ARGUMENTS/context.md`
- Project conventions: `README.md` (and `CLAUDE.md` if present)
- Build note: this project requires CMake flags `-DBUILD_REGION=<REGION> -DBUILD_MAJOR_VERSION=<MAJOR> -DBUILD_MINOR_VERSION=<MINOR>`. The plan/context should specify which region+version combos to build and verify against.

### Step 5 — On completion

After all plan tasks complete and verify, the chosen skill will hand off to `superpowers:finishing-a-development-branch`. Honor that handoff. Then suggest the user run code review:

> All plan tasks complete. Recommend running `superpowers:requesting-code-review` next, which will dispatch the appropriate reviewer agents (plan-adherence) in parallel.

## Important Rules

- Never start implementation on `main`/`master` without explicit user consent.
- Follow plan steps exactly; stop and ask when blocked rather than guessing.
- Run the verification commands the plan specifies; don't claim completion based on assumption.
