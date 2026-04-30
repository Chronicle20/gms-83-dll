---
description: Verify a plan was faithfully implemented — dispatches the plan-adherence-reviewer agent
argument-hint: Task folder name under docs/tasks/ (e.g., "task-003-no-pet-loot")
---

Dispatch the `plan-adherence-reviewer` agent against the task folder: **$ARGUMENTS**.

Pass the task folder path so the agent can locate `plan.md`, run the audit, and write findings to `docs/tasks/$ARGUMENTS/audit.md`.

After the agent completes, summarize the findings to the user — completion rate, blocking issues, and recommended next steps.
