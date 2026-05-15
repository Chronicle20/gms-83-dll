#!/usr/bin/env python3
"""One-shot license-header strip for task-004-code-hygiene.

Walks the repository, matches the leading GNU GPL boilerplate by two anchors
(`This file is part of GMS-83-DLL` and `<https://www.gnu.org/licenses/>`),
and removes the matched comment block plus exactly one trailing newline.

Scope: .cpp .h .cmake .cmake.in below the repository root. Skips .git,
build directories, cmake-build-*, and any common vendor paths. README.md
gets a separate pass to remove the license badge and the trailing
`## License` section.
"""
from __future__ import annotations
import argparse
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

GPL_BLOCK_RE = re.compile(
    r"\A(?:/\*[\s\S]*?\*/\s*\n)",  # leading C-style comment block + trailing newline
)
ANCHOR_OPENER = "This file is part of GMS-83-DLL"
ANCHOR_TRAILER = "<https://www.gnu.org/licenses/>"

EXT_TARGETS = {".cpp", ".h", ".cmake"}
SKIP_DIRS = {".git", "build", "cmake-build-debug", "cmake-build-release",
             "out", "node_modules", "vendor", "third_party", "common/detours"}

def is_target_file(path: Path) -> bool:
    if path.suffix in EXT_TARGETS:
        return True
    if path.name.endswith(".cmake.in"):
        return True
    return False

def strip_block(text: str) -> tuple[str, bool]:
    m = GPL_BLOCK_RE.match(text)
    if not m:
        return text, False
    block = m.group(0)
    if ANCHOR_OPENER not in block or ANCHOR_TRAILER not in block:
        return text, False
    return text[m.end():], True

def strip_readme(text: str) -> str:
    # Remove the GitHub license badge line.
    text = re.sub(r"^!\[GitHub license\][^\n]*\n", "", text, flags=re.M)
    # Remove the trailing `## License` section to EOF.
    text = re.sub(r"\n## License\n[\s\S]*\Z", "\n", text)
    return text

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    changed = 0
    scanned = 0
    for dirpath, dirnames, filenames in os.walk(REPO_ROOT):
        # Prune
        rel = Path(dirpath).relative_to(REPO_ROOT).as_posix()
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS
                       and not (rel + "/" + d).lstrip("/") in SKIP_DIRS]
        for fn in filenames:
            p = Path(dirpath) / fn
            if p.name == "README.md":
                text = p.read_text(encoding="utf-8")
                new = strip_readme(text)
                if new != text:
                    if not args.dry_run:
                        p.write_text(new, encoding="utf-8")
                    print(f"[readme] {p.relative_to(REPO_ROOT)}")
                    changed += 1
                continue
            if not is_target_file(p):
                continue
            scanned += 1
            try:
                text = p.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                continue
            new, did = strip_block(text)
            if did:
                if not args.dry_run:
                    p.write_text(new, encoding="utf-8")
                print(f"[strip ] {p.relative_to(REPO_ROOT)}")
                changed += 1
    print(f"scanned={scanned} changed={changed}", file=sys.stderr)
    return 0

if __name__ == "__main__":
    sys.exit(main())
