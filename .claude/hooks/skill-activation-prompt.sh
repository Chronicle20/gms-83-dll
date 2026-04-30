#!/bin/bash
set -e

# Ensure we run in the hooks dir so relative paths work
cd "$CLAUDE_PROJECT_DIR/.claude/hooks"

# Pipe STDIN to the Python hook (mirrors the original tsx behavior)
python3 skill-activation-prompt.py
