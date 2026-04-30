---
description: Phase 3 — invoke superpowers:writing-plans to produce an implementation plan, using PRD and design as input context
argument-hint: Task folder name under docs/tasks/ (e.g., "task-003-no-pet-loot")
---

You are starting Phase 3 of the four-phase development workflow. The task folder is: **$ARGUMENTS**

## Process

### Step 1 — Validate input

1. Resolve `docs/tasks/$ARGUMENTS/`. Confirm the folder exists.
2. Confirm both `prd.md` and `design.md` exist. If either is missing, stop and tell the user to complete the prior phase first.
3. Confirm `plan.md` does NOT already exist. If it does, ask the user whether to overwrite.

### Step 2 — Load context

Read the following:

- `docs/tasks/$ARGUMENTS/prd.md`
- `docs/tasks/$ARGUMENTS/design.md`
- `README.md` and any `CLAUDE.md` (if present)
- The relevant code areas the design touches (e.g., the analogous existing edit, `common/`, `include/`, the right `memory_maps/<REGION>/v<MAJOR>_<MINOR>.cmake`)

### Step 3 — Invoke the writing-plans skill

Use the Skill tool to invoke `superpowers:writing-plans`. Pass context that:

- The spec is at `docs/tasks/$ARGUMENTS/design.md` (and the PRD at `prd.md` for additional reference).
- The plan output MUST be saved to `docs/tasks/$ARGUMENTS/plan.md` (NOT `docs/superpowers/plans/...`).
- Also produce a `docs/tasks/$ARGUMENTS/context.md` summarizing key files, hook addresses, region/version combos, and dependencies, useful as a quick reference for executing agents.
- After the plan is written and self-reviewed, do NOT proceed to invoke execution. The user will run `/clear` and then `/execute-task $ARGUMENTS` separately.

### Step 4 — Save and confirm

Once `plan.md` and `context.md` are written, tell the user:

> Plan and context saved to `docs/tasks/$ARGUMENTS/`. Now run `/clear` to reset context, then `/execute-task $ARGUMENTS` to begin implementation.

## Important Rules

- DO NOT begin implementation. This phase produces planning documents only.
- DO NOT auto-invoke any execution skill.
- Honor the artifact location override — plan and context live in the task folder.
- Run the `writing-plans` skill's self-review before saving (placeholder scan, type consistency, spec coverage).
