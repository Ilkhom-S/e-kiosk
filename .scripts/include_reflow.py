#!/usr/bin/env python3
"""
include_reflow.py

Reflow and group C/C++ includes in a source or header file.

Features:
- Group includes into sections: Platform (OS), STL, Qt, Modules (Common), ThirdParty, System, Project
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
    """Return category key for an include path.

    Categories: 'stl' (standard headers + windows.h), 'wrapper', 'qt', 'common',
    'sdk', 'thirdparty', 'system', 'project'
    """
    # Common list of C++ standard headers (not exhaustive but covers common cases)
    STD_HEADERS = {
        'algorithm', 'array', 'atomic', 'bitset', 'cassert', 'cctype', 'cerrno', 'cfenv', 'cfloat',
        'chrono', 'cinttypes', 'climits', 'cmath', 'complex', 'condition_variable', 'csetjmp',
        'csignal', 'cstdarg', 'cstdbool', 'cstddef', 'cstdint', 'cstdio', 'cstdlib', 'cstring',
        'ctime', 'cwchar', 'cwctype', 'iomanip', 'iosfwd', 'iostream', 'istream', 'iterator',
        'limits', 'list', 'locale', 'map', 'memory', 'mutex', 'new', 'numeric', 'ostream',
        'queue', 'random', 'ratio', 'regex', 'set', 'sstream', 'stack', 'stdexcept', 'string',
        'strstream', 'system_error', 'tuple', 'typeindex', 'typeinfo', 'type_traits',
        'unordered_map', 'unordered_set', 'utility', 'valarray', 'vector', 'optional', 'variant', 'any'
    }

    lower = path.lower()
    base = os.path.basename(path)
    name, ext = os.path.splitext(base)

    if path.startswith('Common/QtHeadersBegin') or path.startswith('Common/QtHeadersEnd'):
        return 'wrapper'
    # Platform / OS-specific headers should come first (Windows, Winsock, etc.)
    if lower in ('windows.h', 'winsock2.h', 'windowsx.h', 'ws2tcpip.h', 'winsock.h'):
        return 'platform'
    # treat windows and common Win headers as part of the 'stl' group for ordering
    if lower in ('msvcprt.h',):
        return 'stl'
    # simple std header detection: name in STD_HEADERS (without extension)
    if name in STD_HEADERS:
        return 'stl'
    if path.startswith('Qt') or path.startswith('Q'):
        return 'qt'
    if path.startswith('Common/'):
        return 'common'
    if path.startswith('SDK/'):
        return 'sdk'
    if path.startswith('boost/') or path.startswith('thirdparty/'):
        return 'thirdparty'
    # system headers with a slash or typical system names
    if '/' in path or path.startswith('sys/') or path.startswith('unistd'):
        return 'system'
    # fallback: quoted includes and others treated as project
    return 'project'


def reflow_file(contents: str) -> Tuple[str, bool]:
    """Return new contents and whether it changed."""
    lines = contents.splitlines()

    # Find the top block that contains comments, blank lines, and includes
    n = len(lines)
    i = 0
    # skip initial blanks
    while i < n and lines[i].strip() == '':
        i += 1

    # If next lines are comments or includes, treat the continuous block as include region
    start = i
    saw_include_or_comment = False
    while i < n and (lines[i].strip() == '' or lines[i].lstrip().startswith('//') or lines[i].lstrip().startswith('/*') or INCLUDE_RE.match(lines[i])):
        if INCLUDE_RE.match(lines[i]) or lines[i].lstrip().startswith('//'):
            saw_include_or_comment = True
        i += 1

    if not saw_include_or_comment:
        # Try to find an include block after a typical header guard or pragma once
        # Patterns supported: '#pragma once' or '#ifndef ...' '\n' '#define ...'
        j = 0
        if n >= 1 and lines[0].strip().startswith('#pragma once'):
            j = 1
        elif n >= 2 and lines[0].strip().startswith('#ifndef') and lines[1].strip().startswith('#define'):
            j = 2
        else:
            return (contents, False)
        # scan forward from j for a region of includes/comments
        i = j
        saw_include_or_comment = False
        while i < n and (lines[i].strip() == '' or lines[i].lstrip().startswith('//') or lines[i].lstrip().startswith('/*') or INCLUDE_RE.match(lines[i])):
            if INCLUDE_RE.match(lines[i]) or lines[i].lstrip().startswith('//'):
                saw_include_or_comment = True
            i += 1
        if not saw_include_or_comment:
            return (contents, False)
        start = j

    pre = lines[:start]
    region = lines[start:i]
    post = lines[i:]

    includes = []  # tuples (orig_line, path, delim)
    # collect includes from the region (ignore comment lines)
    for ln in region:
        m = INCLUDE_RE.match(ln)
        if m:
            path = m.group(1) or m.group(2)
            delim = '<' if m.group(1) else '"'
            includes.append((ln, path, delim))

    # If no includes, return original
    if not includes:
        return (contents, False)

    # Preserve leading comments (block or line comments) at the start of the region
    leading_comments: List[str] = []
    GROUP_NAMES = ['Platform', 'STL', 'Qt', 'Modules', 'SDK', 'ThirdParty', 'System', 'Project']
    GROUP_COMMENT_RE = re.compile(r'^\s*//\s*(?:' + '|'.join(GROUP_NAMES) + r')\s*$')
    for ln in region:
        if INCLUDE_RE.match(ln):
            break
        # preserve blank lines and comment lines only, but skip existing group comment headers
        if ln.strip() == '' or ln.lstrip().startswith('/*'):
            leading_comments.append(ln)
        elif ln.lstrip().startswith('//'):
            if not GROUP_COMMENT_RE.match(ln):
                leading_comments.append(ln)
            else:
                # skip existing group comment like '// Qt' to avoid duplicates
                continue
        else:
            # non-comment content encountered before includes; do not preserve further
            break

    # Categorize
    groups = {'platform': [], 'stl': [], 'qt': [], 'common': [], 'thirdparty': [], 'system': [], 'project': [], 'wrapper': []}
    for orig, path, delim in includes:
        cat = categorize_include(path)
        groups.setdefault(cat, []).append((orig, path, delim))

    out_lines: List[str] = []

    # Preserve any 'pre' content (e.g., header guard) that appeared before the include region
    if pre:
        out_lines.extend(pre)
        if out_lines and out_lines[-1].strip():
            out_lines.append('')

    # keep a short comment explaining this section
    def emit_group(comment: str, items: List[Tuple[str,str,str]], wrapper: bool=False):
        if not items:
            return
        out_lines.append(f'// {comment}')
        if wrapper and items:
            out_lines.append('#include <Common/QtHeadersBegin.h>')
        # custom sort: project includes from current folder (no '/') should go last
        if comment == 'Project':
            items_sorted = sorted(items, key=lambda t: (0 if '/' in t[1] else 1, t[1]))
        else:
            items_sorted = sorted(items, key=lambda t: t[1])
        for _, path, delim in items_sorted:
            if delim == '<':
                out_lines.append(f'#include <{path}>')
            else:
                out_lines.append(f'#include "{path}"')
        if wrapper and items:
            out_lines.append('#include <Common/QtHeadersEnd.h>')
        out_lines.append('')

    # If the region started with comments (e.g., top-of-file block comment), preserve them
    if 'leading_comments' in locals() and leading_comments:
        # Avoid adding duplicate blank lines if pre already ends with a blank
        if leading_comments[0].strip() == '' and out_lines and out_lines[-1].strip() == '':
            out_lines.extend(leading_comments[1:])
        else:
            out_lines.extend(leading_comments)
        # ensure a separating blank line between preserved comments and the grouped includes
        if not out_lines[-1].strip():
            # already ends with blank line
            pass
        else:
            out_lines.append('')

    # Order: Platform (OS-specific), STL, Qt (with wrapper), Modules, SDK, ThirdParty, System, Project
    emit_group('Platform', groups.get('platform', []))
    emit_group('STL', groups.get('stl', []))
    emit_group('Qt', groups.get('qt', []), wrapper=True)
    emit_group('Modules', groups.get('common', []))
    emit_group('SDK', groups.get('sdk', []))
    emit_group('ThirdParty', groups.get('thirdparty', []))
    emit_group('System', groups.get('system', []))
    emit_group('Project', groups.get('project', []))

    # If there's no post content, avoid leaving a trailing blank line after the last group
    if not post and out_lines and out_lines[-1] == '':
        out_lines.pop()

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
