---
description: Phase 2 — invoke superpowers:brainstorming to produce a design doc, using an existing PRD as input context
argument-hint: Task folder name under docs/tasks/ (e.g., "task-003-no-pet-loot")
---

You are starting Phase 2 of the four-phase development workflow. The task folder is: **$ARGUMENTS**

## Process

### Step 1 — Validate input

1. Treat `$ARGUMENTS` as a folder name under `docs/tasks/`. Resolve to `docs/tasks/$ARGUMENTS/`.
2. Confirm the folder exists. If not, stop and ask the user for the correct task folder name.
3. Confirm `prd.md` exists in that folder. If not, stop and tell the user to run `/spec-task` first or provide the missing PRD.
4. Confirm `design.md` does NOT already exist. If it does, ask the user whether to overwrite or open the existing one.

### Step 2 — Load context

Read the following:

- `docs/tasks/$ARGUMENTS/prd.md` (the requirements being designed against)
- `README.md` and any project-level `CLAUDE.md` (if present) for project conventions
- The most analogous existing edit (e.g., `bypass/`, `redirect/`, `no-ad-balloon/`) named in the PRD's "Affected components" section
- Relevant pieces of `common/` and `include/` based on the PRD's Hook / Patch Surface

### Step 3 — Invoke the brainstorming skill

Use the Skill tool to invoke `superpowers:brainstorming`. Pass context that:

- The PRD has already been written and approved at `docs/tasks/$ARGUMENTS/prd.md`.
- The brainstorming skill should SKIP its default what/why questions (purpose, success criteria) because the PRD answers them.
- Focus questions on hook strategy (IAT vs detour vs inline patch), alternatives, and tradeoffs (stability, AV/Themida visibility, region/version portability).
- The design output MUST be saved to `docs/tasks/$ARGUMENTS/design.md` (NOT the skill's default `docs/superpowers/specs/...` location).
- After the design is approved by the user, do NOT proceed to invoke `writing-plans`. The user will run `/clear` and then `/plan-task $ARGUMENTS` separately, in a fresh session.

### Step 4 — Save and confirm

Once the brainstorming skill has produced an approved design, ensure it is written to `docs/tasks/$ARGUMENTS/design.md`. Then tell the user:

> Design saved to `docs/tasks/$ARGUMENTS/design.md`. Now run `/clear` to reset context, then `/plan-task $ARGUMENTS` to produce the implementation plan.

## Important Rules

- DO NOT begin implementation. This phase produces a design document only.
- DO NOT auto-invoke `writing-plans`. The `/clear` between phases is intentional.
- Honor the artifact location override — the design lives in the task folder, not the superpowers default location.
