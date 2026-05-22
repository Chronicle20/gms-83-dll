#!/usr/bin/env python3
"""Convert IDA Pro `analyze_struct_detailed` JSON output to a Markdown reference table.

Usage:
  ida_struct_to_md.py <path-to-json>        # read from file
  ida_struct_to_md.py -                     # read JSON from stdin
  cat foo.json | ida_struct_to_md.py        # via pipe

Flags:
  --collapse-tears   Collapse `_ZtlSecureTear_X` payload + `_X_CS` cookie pairs
                     into a single logical row (`SecureTear<T> X`, 12 bytes).
  --title TITLE      Override the inferred H1 title (default: struct name).
  --no-summary       Skip the "Total size / Member count" header block.
  --no-id-column     Omit the leading "#" index column.

Examples:
  # Plain pass-through to markdown
  ida_struct_to_md.py CWvsContext.json > v95_reference.md

  # SecureTear-heavy struct, want pair-collapsed view
  ida_struct_to_md.py --collapse-tears SecondaryStat.json > v95_secondarystat.md

This script is one half of the version-porting workflow documented at
docs/version-porting-workflow.md — feed it the JSON from IDA's
`analyze_struct_detailed` MCP tool to produce the canonical v95 reference for a
struct, then verify against the target version via disassembly.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from typing import Any, Iterator


def _parse_offset(s: str) -> int:
    """IDA emits offsets as hex strings like '0x00000020'."""
    return int(s, 16) if isinstance(s, str) and s.lower().startswith("0x") else int(s)


def _emit_row(cols: list[str]) -> str:
    return "| " + " | ".join(cols) + " |"


def _collapse_secure_tears(members: list[dict[str, Any]]) -> Iterator[dict[str, Any]]:
    """Walk members and merge `_ZtlSecureTear_X` payload+CS pairs.

    Members are emitted in original order. A pair is detected when:
      - The current member's name starts with `_ZtlSecureTear_`
      - The next member's name is the current name + `_CS`
      - Both are at consecutive offsets (payload_off + payload_size == cs_off)
    Otherwise members are emitted unchanged.
    """
    i = 0
    while i < len(members):
        m = members[i]
        name = m.get("name", "")
        if name.startswith("_ZtlSecureTear_") and not name.endswith("_CS") and i + 1 < len(members):
            n = members[i + 1]
            n_name = n.get("name", "")
            if n_name == f"{name}_CS":
                payload_off = _parse_offset(m["offset"])
                payload_size = int(m["size"])
                cs_off = _parse_offset(n["offset"])
                if payload_off + payload_size == cs_off:
                    # Detect template arg from payload type: int[2] → long, short[2] → short, etc.
                    payload_type = m.get("type", "")
                    tparam = _infer_secure_tear_template(payload_type)
                    yield {
                        "offset": m["offset"],
                        "size": payload_size + int(n["size"]),
                        "type": f"SecureTear<{tparam}>",
                        "name": name.removeprefix("_ZtlSecureTear_"),
                        "is_nested_udt": False,
                    }
                    i += 2
                    continue
        yield m
        i += 1


def _infer_secure_tear_template(payload_type: str) -> str:
    """Map IDA payload types like `int[2]` to a probable `<T>` template arg."""
    # Common cases observed in MapleStory IDBs.
    table = {
        "int[2]": "long",
        "unsigned int[2]": "unsigned long",
        "short[2]": "short",
        "__int16[2]": "short",
        "char[2]": "char",
        "long[2]": "long",
    }
    return table.get(payload_type, payload_type)


def to_markdown(
    struct: dict[str, Any],
    *,
    collapse_tears: bool = False,
    title: str | None = None,
    include_summary: bool = True,
    include_id_column: bool = True,
) -> str:
    name = struct.get("name", "UNKNOWN")
    total_size = struct.get("size", "?")
    members = struct.get("members", [])

    if collapse_tears:
        members = list(_collapse_secure_tears(members))

    lines: list[str] = []
    h1 = title if title is not None else f"`{name}` reference layout"
    lines.append(f"# {h1}")
    lines.append("")

    if include_summary:
        is_union = struct.get("is_union", False)
        type_kind = "Union" if is_union else "Struct"
        member_count = struct.get("member_count", len(members))
        kind_detail = struct.get("udt_type", type_kind)
        lines.append(f"**Type kind:** {kind_detail}  ")
        lines.append(f"**Total size:** {total_size} bytes")
        if isinstance(total_size, int):
            lines[-1] += f" (0x{total_size:X})"
        lines[-1] += "  "
        lines.append(f"**Members:** {member_count} (raw IDA count)")
        if collapse_tears:
            lines[-1] += f" / {len(members)} (after SecureTear collapse)"
        lines.append("")

    # Header row
    headers = ["Offset", "Size", "Type", "Name"]
    if include_id_column:
        headers = ["#"] + headers
    lines.append(_emit_row(headers))
    lines.append(_emit_row(["---"] * len(headers)))

    for idx, m in enumerate(members):
        off_raw = m.get("offset", "0x0")
        off_int = _parse_offset(off_raw) if isinstance(off_raw, str) else int(off_raw)
        size = m.get("size", "?")
        mtype = m.get("type", "?")
        mname = m.get("name", "")
        # Empty name (anon nested UDT / EBCO base) → render as italics with type
        display_name = f"`{mname}`" if mname else f"*(anon {mtype})*"
        row = [
            f"0x{off_int:04X}",
            str(size),
            f"`{mtype}`",
            display_name,
        ]
        if include_id_column:
            row = [str(idx)] + row
        lines.append(_emit_row(row))

    lines.append("")
    return "\n".join(lines)


def _read_json(src: str) -> dict[str, Any]:
    raw = sys.stdin.read() if src == "-" else open(src, encoding="utf-8").read()
    # IDA MCP sometimes wraps results in extra envelopes; tolerate `{"result":{...}}`.
    data = json.loads(raw)
    if isinstance(data, dict) and "members" not in data and "result" in data:
        data = data["result"]
    if not isinstance(data, dict) or "members" not in data:
        raise SystemExit("Input doesn't look like an analyze_struct_detailed JSON object")
    return data


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument("source", help="JSON path, or '-' for stdin")
    p.add_argument("--collapse-tears", action="store_true",
                   help="collapse _ZtlSecureTear_X + _X_CS pairs into one row")
    p.add_argument("--title", help="override the H1 title")
    p.add_argument("--no-summary", action="store_true",
                   help="skip the size/member-count header block")
    p.add_argument("--no-id-column", action="store_true",
                   help="omit the leading index column")
    args = p.parse_args(argv)

    struct = _read_json(args.source)
    md = to_markdown(
        struct,
        collapse_tears=args.collapse_tears,
        title=args.title,
        include_summary=not args.no_summary,
        include_id_column=not args.no_id_column,
    )
    sys.stdout.write(md)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
