#!/usr/bin/env bash
# PreToolUse hook: blocks `git push` when clang-format-18 has suggestions on
# PR-introduced C++ lines. Mirrors the check in .github/workflows/lint.yml so
# formatting failures get caught locally instead of round-tripping through CI.
#
# Allow-path: not a `git push`, no clang-format-diff installed, no diff vs
# origin/main, or clang-format finds nothing to change — exit 0, silent.
# Deny-path: clang-format produces a diff — emit hookSpecificOutput with
# permissionDecision=deny and the verbatim diff in permissionDecisionReason.

set -u

INPUT=$(cat)
COMMAND=$(echo "$INPUT" | jq -r '.tool_input.command // empty')

# Only act on `git push`. Substring match catches chained forms like
# `git fetch && git push` that a prefix `if`-filter would miss.
case "$COMMAND" in
    *"git push"*) ;;
    *) exit 0 ;;
esac

PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$(pwd)}"
CLANG_FORMAT_DIFF="${HOME}/.local/bin/clang-format-diff.py"

# If the tooling isn't installed, don't block — fail open.
if [ ! -f "$CLANG_FORMAT_DIFF" ]; then
    exit 0
fi

cd "$PROJECT_DIR" || exit 0

# Ensure origin/main exists locally so the three-dot diff has a valid base.
# Silent on failure (e.g. offline, no remote) — the diff itself will fail
# gracefully below.
git fetch origin main --quiet 2>/dev/null || true

# If origin/main doesn't resolve (no remote, fresh clone), there's nothing to
# diff against — let the push through.
if ! git rev-parse --verify --quiet origin/main >/dev/null; then
    exit 0
fi

RESULT=$(git diff -U0 origin/main...HEAD -- '*.cpp' '*.h' '*.hpp' 2>/dev/null \
         | python3 "$CLANG_FORMAT_DIFF" -p1 -style=file 2>/dev/null)

if [ -z "$RESULT" ]; then
    # No suggestions — clean push.
    exit 0
fi

# clang-format-diff produced output. Deny the push and surface the diff so
# Claude can apply the fixes.
REASON="clang-format-18 has suggestions on PR-introduced C++ lines (this mirrors .github/workflows/lint.yml — it will fail CI). Fix the lines below before pushing:

$RESULT"

jq -n --arg reason "$REASON" '{
    hookSpecificOutput: {
        hookEventName: "PreToolUse",
        permissionDecision: "deny",
        permissionDecisionReason: $reason
    }
}'
