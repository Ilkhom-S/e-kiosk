"""
Simple include reflow utility used by unit tests.
Provides: reflow_file(input_contents: str) -> (new_contents: str, changed: bool)

Rules implemented (minimal, test-focused):
- Groups: Platform, STL, Qt, Modules, SDK, ThirdParty, System, Project
- Qt includes are wrapped with Common/QtHeadersBegin.h / Common/QtHeadersEnd.h
- Includes inside each group are sorted alphabetically and duplicates are removed
- Top block comment (//... or /* ... */) is preserved
- Non-include tail (other code) is preserved after grouped includes

This is intentionally small and only implements the behaviour required by
existing unit tests in tests/scripts/test_include_reflow.py.
"""
from __future__ import annotations
import re
from typing import List, Tuple

INCLUDE_RE = re.compile(r'^\s*#include\s*(<|\")([^>\"]+)(>|\")\s*$', flags=re.M)

PLATFORM_HDRS = {"windows.h"}
SYSTEM_PREFIXES = ("sys/",)


def _categorize(path: str) -> str:
    # project includes - quoted form handled by caller
    if path.startswith("Qt") or "/Qt" in path or path.startswith("Qt"):
        return "Qt"
    if path in PLATFORM_HDRS:
        return "Platform"
    if "/" not in path:
        # heuristics: no slash and not a module -> STL
        return "STL"
    first = path.split("/")[0]
    if first == "Common":
        return "Modules"
    if first == "SDK":
        return "SDK"
    if first in ("sys",):
        return "System"
    # third-party (e.g. boost/..., openssl/..., etc.)
    return "ThirdParty"


def _format_group(name: str, items: List[str]) -> List[str]:
    if not items:
        return []
    out: List[str] = []
    out.append(f"// {name}")
    if name == "Qt":
        out.append("#include <Common/QtHeadersBegin.h>")
        for inc in items:
            out.append(f"#include <{inc}>")
        out.append("#include <Common/QtHeadersEnd.h>")
    else:
        for inc in items:
            # Project headers are quoted by caller
            if name == "Project":
                out.append(f"#include \"{inc}\"")
            else:
                out.append(f"#include <{inc}>")
    return out


def reflow_file(input_contents: str) -> Tuple[str, bool]:
    lines = input_contents.splitlines()

    # Preserve leading block comment (// or /* */) if present
    leading: List[str] = []
    i = 0
    if i < len(lines):
        if lines[i].strip().startswith("//") or lines[i].strip().startswith("/*"):
            # capture until first blank line after comment block
            while i < len(lines) and lines[i].strip() != "":
                leading.append(lines[i])
                i += 1
            # consume a single separating blank line
            if i < len(lines) and lines[i].strip() == "":
                i += 1

    include_lines = []
    tail_lines = []
    # collect includes and tail
    while i < len(lines):
        m = INCLUDE_RE.match(lines[i])
        if m:
            include_lines.append(m.group(2).strip())
        else:
            tail_lines = lines[i:]
            break
        i += 1

    # categorize includes
    groups = {
        "Platform": [],
        "STL": [],
        "Qt": [],
        "Modules": [],
        "SDK": [],
        "ThirdParty": [],
        "System": [],
        "Project": [],
    }

    for inc in include_lines:
        # detect quoted project includes
        if inc.startswith('"') or inc.endswith('"'):
            groups["Project"].append(inc.strip('"'))
            continue
        # For angle-bracket includes, categorize
        cat = _categorize(inc)
        groups[cat].append(inc)

    # sort & deduplicate each group
    for k in groups.keys():
        unique = sorted(set(groups[k]))
        groups[k] = unique

    order = ["Platform", "STL", "Qt", "Modules", "SDK", "ThirdParty", "System", "Project"]

    out_lines: List[str] = []
    if leading:
        out_lines.extend(leading)
        out_lines.append("")

    first_group = True
    for name in order:
        formatted = _format_group(name, groups[name])
        if not formatted:
            continue
        if not first_group:
            out_lines.append("")
        out_lines.extend(formatted)
        first_group = False

    # Add a separating blank line before project code/tail if any
    if tail_lines and (not tail_lines[0].strip() == ""):
        out_lines.append("")

    out_lines.extend(tail_lines)

    # Ensure trailing newline
    new_contents = "\n".join(out_lines)
    if not new_contents.endswith("\n"):
        new_contents += "\n"

    changed = (new_contents != input_contents)
    return new_contents, changed


if __name__ == "__main__":
    import sys
    txt = sys.stdin.read()
    out, ch = reflow_file(txt)
    sys.stdout.write(out)
