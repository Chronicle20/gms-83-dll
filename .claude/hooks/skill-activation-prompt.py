#!/usr/bin/env python3
"""
Claude skill-activation hook (Python port).

Reads JSON from STDIN (mirrors your TS hook's input shape) and prints either:
- A grouped "Skill Activation" banner for suggest/warn matches, or
- A BLOCK message and exits non-zero for guardrails with enforcement="block"
  unless skip conditions are met.

INPUT (JSON via stdin)  — compatible with your TS hook:
{
  "session_id": "...",
  "transcript_path": "...",
  "cwd": "...",

  "permission_mode": "...",
  "prompt": "user text",
  "meta": {
    "file": "apps/web/src/App.tsx",          # optional path for fileTriggers
    "fileContent": "...",                    # optional content for contentPatterns
    "branch": "feat/..."                     # unused here but allowed
  }
}

ENV:
- CLAUDE_PROJECT_DIR: repo root (where .claude/skills/skill-rules.json lives)
- SKIP_<ENV_OVERRIDE>=1                     # e.g., SKIP_FRONTEND_GUIDELINES=1
- SKILL_USED: comma-separated skill names used in this session (for sessionSkillUsed skip)
"""

import os, sys, json, re, fnmatch
from pathlib import Path
from typing import Any, Dict, List, Tuple

RULES_REL_PATH = ".claude/skills/skill-rules.json"


def read_stdin_json() -> Dict[str, Any]:
    raw = sys.stdin.read().strip()
    if not raw:
        return {}
    try:
        return json.loads(raw)
    except Exception as e:

        print(f"Invalid JSON on stdin: {e}", file=sys.stderr)
        return {}



def load_rules() -> Dict[str, Any]:
    base = os.environ.get("CLAUDE_PROJECT_DIR") or os.getcwd()
    path = Path(base) / RULES_REL_PATH

    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def text_contains_any(text: str, terms: List[str]) -> bool:
    t = text.lower()
    return any(term.lower() in t for term in terms)


def regex_any(text: str, patterns: List[str]) -> bool:

    for patt in patterns:
        try:
            if re.search(patt, text, flags=re.IGNORECASE | re.MULTILINE):

                return True
        except re.error:
            pass
    return False



def path_matches_any(path: str, patterns: List[str]) -> bool:
    return any(fnmatch.fnmatch(path, pat) for pat in patterns)


def get_file_text(file_path: str) -> str:

    try:
        with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
            return f.read()
    except Exception:
        return ""


def should_skip_block(
    skill_name: str, skip_cfg: Dict[str, Any], file_text: str
) -> bool:
    # envOverride — if set, allow skip

    env_override = skip_cfg.get("envOverride")

    if env_override:
        if os.environ.get(env_override, "").strip().lower() in (
            "1",
            "true",
            "yes",
            "on",

        ):
            return True

    # sessionSkillUsed — if set true, and SKILL_USED contains name, allow skip
    if skip_cfg.get("sessionSkillUsed"):
        used = os.environ.get("SKILL_USED", "")

        used_set = {x.strip() for x in used.split(",") if x.strip()}
        if skill_name in used_set:
            return True


    # fileMarkers — if any sentinel exists in file content, allow skip
    file_markers = skip_cfg.get("fileMarkers") or []
    if file_text and any(marker in file_text for marker in file_markers):
        return True

    return False


def match_skill(
    skill_name: str, cfg: Dict[str, Any], prompt: str, meta: Dict[str, Any]
) -> Tuple[bool, Dict[str, str]]:
    """
    Returns (matched?, details) where details may contain:
     - matchType: 'keyword' | 'intent' | 'file'
     - reason: short text
    """
    prompt_trig = cfg.get("promptTriggers") or {}
    file_trig = cfg.get("fileTriggers") or {}

    # Prompt keyword/intent matching (skill-rules.json: promptTriggers.keywords/intentPatterns)
    # e.g., backend terms, React/MUI, etc.
    if prompt_trig:
        kws = prompt_trig.get("keywords") or []
        if kws and text_contains_any(prompt, kws):
            return True, {"matchType": "keyword", "reason": "keyword match"}

        intents = prompt_trig.get("intentPatterns") or []

        if intents and regex_any(prompt, intents):
            return True, {"matchType": "intent", "reason": "intent pattern"}

    # File-based matching (skill-rules.json: fileTriggers.pathPatterns/pathExclusions/contentPatterns)
    file_path = (meta.get("file") or "").strip()
    file_content = meta.get("fileContent") or ""

    if file_trig and file_path:
        pp = file_trig.get("pathPatterns") or []
        px = file_trig.get("pathExclusions") or []

        cp = file_trig.get("contentPatterns") or []

        # path include/exclude
        if pp and path_matches_any(file_path, pp):
            if px and path_matches_any(file_path, px):
                pass  # excluded — keep checking other triggers (maybe prompt matched earlier)
            else:
                # optional content fingerprints
                content_ok = True
                if cp:
                    # Ensure we have content to test

                    if not file_content:
                        file_content = get_file_text(file_path)
                    content_ok = regex_any(file_content, cp)
                if content_ok:
                    return True, {
                        "matchType": "file",

                        "reason": "path/content patterns",
                    }

    return False, {}


def group_by_priority(
    matches: List[Tuple[str, Dict[str, Any]]],

) -> Dict[str, List[str]]:
    buckets = {"critical": [], "high": [], "medium": [], "low": []}
    for name, cfg in matches:
        pr = (cfg.get("priority") or "low").lower()
        if pr not in buckets:
            pr = "low"
        buckets[pr].append(name)

    return buckets



def main():
    data = read_stdin_json()
    prompt = (data.get("prompt") or "").strip()
    meta = dict(data.get("meta") or {})
    if not prompt:
        # No prompt — do nothing (match TS behavior: silent success)
        sys.exit(0)

    rules = load_rules()
    skills: Dict[str, Any] = rules.get("skills") or {}

    matched: List[Tuple[str, Dict[str, Any], Dict[str, str]]] = []
    for name, cfg in skills.items():
        ok, details = match_skill(name, cfg, prompt, meta)
        if ok:
            matched.append((name, cfg, details))


    if not matched:
        # No output when nothing matches (parity with your TS style)
        sys.exit(0)

    # If any blocking guardrail matches and isn't skipped, print its blockMessage and exit 3.
    # (frontend-dev-guidelines has enforcement="block" and a blockMessage/skipConditions).
    # See your JSON’s fields.
    #   enforcement: "block" | "suggest" | "warn"  (values documented under notes)
    #   blockMessage, skipConditions.sessionSkillUsed/fileMarkers/envOverride
    # References: skill JSON shows these fields.  # noqa
    # (We intentionally evaluate ALL matched skills; the first active guardrail triggers the block.)
    for name, cfg, _ in matched:
        if (cfg.get("enforcement") or "").lower() == "block":
            skip_cfg = cfg.get("skipConditions") or {}
            file_path = (meta.get("file") or "").strip()
            file_text = meta.get("fileContent") or (
                get_file_text(file_path) if file_path else ""
            )
            if should_skip_block(name, skip_cfg, file_text):
                continue  # skip blocking; allow banner to appear below
            block_msg = cfg.get("blockMessage") or "Blocked by guardrail."
            # token replace {file_path} as in your rule
            block_msg = block_msg.replace("{file_path}", file_path or "<unknown>")
            print(block_msg)
            sys.exit(3)

    # Otherwise, produce a grouped activation banner like the TS script
    buckets = group_by_priority([(n, c) for (n, c, _) in matched])

    out = []
    out.append("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    out.append("🎯 SKILL ACTIVATION CHECK")
    out.append("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    out.append("")

    if buckets["critical"]:
        out.append("⚠️ CRITICAL SKILLS (REQUIRED):")
        for n in buckets["critical"]:
            out.append(f"  → {n}")
        out.append("")

    if buckets["high"]:
        out.append("📚 RECOMMENDED SKILLS:")
        for n in buckets["high"]:
            out.append(f"  → {n}")
        out.append("")

    if buckets["medium"]:
        out.append("💡 SUGGESTED SKILLS:")
        for n in buckets["medium"]:
            out.append(f"  → {n}")
        out.append("")

    if buckets["low"]:
        out.append("📌 OPTIONAL SKILLS:")
        for n in buckets["low"]:
            out.append(f"  → {n}")
        out.append("")

    out.append("ACTION: Use Skill tool BEFORE responding")
    out.append("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print("\n".join(out))
    sys.exit(0)


if __name__ == "__main__":
    main()
