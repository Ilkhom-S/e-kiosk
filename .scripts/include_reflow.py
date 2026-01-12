#!/usr/bin/env python3
"""
include_reflow.py

Reflow and group C/C++ includes in a source or header file.

Features:
- Group includes into sections: Qt, Modules (Common), ThirdParty, System, Project
- Ensure all Qt includes are wrapped between <Common/QtHeadersBegin.h> and <Common/QtHeadersEnd.h>
- Insert simple comment headers before each group (e.g. // Qt)
- Dry-run mode (default) prints a unified diff; --apply writes changes in-place

Usage:
  python ./.scripts/include_reflow.py [--apply] <file1> [file2 ...]

Exit codes:
  0: success (no changes or changes applied)
  2: some files were changed in dry-run (non-zero diff)
"""
from __future__ import annotations

import argparse
import os
import re
import sys
import difflib
from typing import List, Tuple


INCLUDE_RE = re.compile(r'^\s*#\s*include\s*(?:<([^>]+)>|"([^"]+)")')


def categorize_include(path: str) -> str:
    """Return category key for an include path."""
    if path.startswith('Common/QtHeadersBegin') or path.startswith('Common/QtHeadersEnd'):
        return 'wrapper'
    if path.startswith('Qt') or path.startswith('QtCore') or path.startswith('QtGui') or path.startswith('QtNetwork') or path.startswith('Q'):
        return 'qt'
    if path.startswith('Common/'):
        return 'common'
    if path.startswith('boost/'):
        return 'thirdparty'
    if '/' in path or path.startswith('sys/') or path.startswith('unistd'):
        return 'system'
    # fallback: if it's quoted include it's project
    return 'project'


def reflow_file(contents: str) -> Tuple[str, bool]:
    """Return new contents and whether it changed."""
    lines = contents.splitlines()

    pre = []
    includes = []  # tuples (orig_line, path, delim)
    post = []
    in_includes = True

    for ln in lines:
        m = INCLUDE_RE.match(ln)
        if in_includes and m:
            path = m.group(1) or m.group(2)
            delim = '<' if m.group(1) else '"'
            includes.append((ln, path, delim))
        else:
            # Once we hit a non-include line, everything after is post
            in_includes = False
            post.append(ln)

    # If no includes, return original
    if not includes:
        return (contents, False)

    # Categorize
    groups = {'qt': [], 'common': [], 'thirdparty': [], 'system': [], 'project': [], 'wrapper': []}
    for orig, path, delim in includes:
        cat = categorize_include(path)
        groups.setdefault(cat, []).append((orig, path, delim))

    out_lines: List[str] = []
    # keep a short comment explaining this section
    def emit_group(comment: str, items: List[Tuple[str,str,str]], wrapper: bool=False):
        if not items:
            return
        out_lines.append(f'// {comment}')
        if wrapper:
            out_lines.append('#include <Common/QtHeadersBegin.h>')
        # sort items by path
        items_sorted = sorted(items, key=lambda t: t[1])
        for _, path, delim in items_sorted:
            if delim == '<':
                out_lines.append(f'#include <{path}>')
            else:
                out_lines.append(f'#include "{path}"')
        if wrapper:
            out_lines.append('#include <Common/QtHeadersEnd.h>')
        out_lines.append('')

    # Order: Qt (with wrapper), Common (modules), ThirdParty, System, Project
    emit_group('Qt', groups.get('qt', []), wrapper=True)
    emit_group('Modules', groups.get('common', []))
    emit_group('ThirdParty', groups.get('thirdparty', []))
    emit_group('System', groups.get('system', []))
    emit_group('Project', groups.get('project', []))

    # Append the rest of the file (skip leading blank if present)
    # Remove leading blank lines from post
    while post and post[0].strip() == '':
        post.pop(0)
    if post:
        out_lines.extend(post)

    new_contents = '\n'.join(out_lines) + ('\n' if contents.endswith('\n') else '')
    return (new_contents, new_contents != contents)


def process_path(path: str, apply: bool=False) -> int:
    if not os.path.isfile(path):
        print(f'Skipping non-file: {path}')
        return 0
    with open(path, 'r', encoding='utf-8', errors='surrogateescape') as f:
        contents = f.read()
    new_contents, changed = reflow_file(contents)
    if not changed:
        print(f'No change: {path}')
        return 0
    if apply:
        with open(path, 'w', encoding='utf-8', errors='surrogateescape') as f:
            f.write(new_contents)
        print(f'Updated: {path}')
        return 0
    # dry-run: show diff
    diff = difflib.unified_diff(contents.splitlines(keepends=True), new_contents.splitlines(keepends=True), fromfile=path, tofile=path+' (reflowed)')
    sys.stdout.writelines(diff)
    return 2


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('--apply', action='store_true', help='Apply changes in-place')
    parser.add_argument('files', nargs='+', help='Files to process')
    args = parser.parse_args(argv)

    rc = 0
    for p in args.files:
        r = process_path(p, apply=args.apply)
        if r != 0:
            rc = r
    return rc


if __name__ == '__main__':
    raise SystemExit(main(sys.argv[1:]))
