---
description: Turn a backlog idea into a structured feature spec with a PRD and supporting materials
argument-hint: Brief description of the feature idea (e.g., "no-pet-loot", "auto-login-skip")
---

You are a product-minded engineer helping turn a rough backlog idea into a well-structured feature spec. The idea is: **$ARGUMENTS**

## Process

### Step 1 — Determine Task Number

Look at existing folders in `docs/tasks/` to find the next available task number. Task folders use the format `task-NNN-slug` (e.g., `task-001-init`, `task-002-no-pet-loot`). Derive the slug from the idea name (lowercase, hyphenated, max 3-4 words). If `docs/tasks/` does not yet exist, create it and start at `task-001`.

### Step 2 — Understand Context

Before writing anything, gather context:

1. Read `README.md` and any docs under `docs/` to understand what the project is and what edits already exist.
2. Scan the codebase to understand which existing edit (e.g., `bypass/`, `redirect/`, `no-ad-balloon/`) is most analogous, and what shared utilities live in `common/` and `include/`.
3. Look at `CMakeLists.txt` and the per-edit `CMakeLists.txt` files to understand how new DLL edits are wired in.
4. Identify whether the feature is region/version-specific (memory map dependent) or generic.

### Step 3 — Collaborate on the Spec

Do NOT immediately generate all files. Instead, present the user with:

1. **Scope summary** — Your understanding of the feature in 2-3 sentences
2. **Key questions** — Anything ambiguous or requiring a decision (list 3-7 questions; e.g., target region/version, IAT hook vs detour, INI config surface, default on/off)
3. **Proposed boundaries** — What's in scope vs explicitly out of scope
4. **Affected components** — Which existing edit(s) this resembles, what `common/`/`include/` additions are needed, whether new memory map entries are required

Wait for the user to answer questions and confirm scope before proceeding.

### Step 4 — Generate the Task Folder

Once scope is confirmed, create the task folder at `docs/tasks/task-NNN-slug/` with these files:

#### `prd.md` — Product Requirements Document

Use this structure:

```markdown
# [Feature Name] — Product Requirements Document

Version: v1
Status: Draft
Created: YYYY-MM-DD
---

## 1. Overview

[What this client edit does and why it matters — 2-3 paragraphs]

## 2. Goals

Primary goals:
- [list]

Non-goals:
- [list]

## 3. User Stories

- As a [server operator / player / developer], I want to [action] so that [outcome]
- [repeat]

## 4. Functional Requirements

[Organized by capability area. Be specific and testable.]

## 5. Hook / Patch Surface

[What the DLL hooks or patches: function addresses, IAT entries, opcodes, message handlers. Include region/version applicability.]

## 6. Configuration

[INI config keys exposed (if any), defaults, valid ranges]

## 7. Memory Map Impact

[New entries needed in memory_maps/<REGION>/v<MAJOR>_<MINOR>.cmake, or confirmation that none are needed]

## 8. Non-Functional Requirements

[Stability, side effects on unrelated client behavior, AV/Themida compatibility, performance]

## 9. Open Questions

[Anything still unresolved after the conversation]

## 10. Acceptance Criteria

[Concrete checklist of what "done" looks like — including which region+version combos must work]
```

#### Supporting Materials (create as needed)

Depending on the feature, also create relevant supporting files:

- `hook-design.md` — Detailed hook/detour/IAT specifications if non-trivial
- `memory-map.md` — Address tables across regions/versions if the feature is broadly applicable
- `config-schema.md` — INI schema with examples if config is rich
- `risks.md` — If there are significant stability or detection risks worth documenting separately

Only create supporting files that add value beyond what's in the PRD. Don't create empty or repetitive files.

### Step 5 — Summary

After generating files, present:

1. List of files created with brief description of each
2. Suggested next step: "Now run `/clear` to reset context, then `/design-task task-NNN-slug` to invoke the brainstorming/design phase"

## Quality Standards

- Requirements must be specific and testable — avoid vague language like "should be fast" or "should not crash too much"
- Always state the target region+version combos explicitly; "GMS v83.1" is testable, "the current client" is not
- Hook specs must include enough address/signature detail that the implementing engineer can locate the patch site without re-reverse-engineering
- Keep the PRD self-contained — a developer should be able to implement from it without needing to ask clarifying questions
