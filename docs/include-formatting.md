**Include Formatting & Wrapper Policy**

This doc explains the project's include grouping policy, the `Common/QtHeadersBegin.h`/`Common/QtHeadersEnd.h` wrapper requirement, and how to integrate the provided reflow script into editors and Git hooks.

Purpose

- Keep all Qt includes grouped and wrapped between `Common/QtHeadersBegin.h` and `Common/QtHeadersEnd.h`.
- Group includes into named sections with small comment headers so files read naturally (e.g. `// Qt`, `// Modules`, `// Project`).
- Apply deterministic sorting so diffs are minimal and reviews are easy.

Provided tool

- `.scripts/include_reflow.py` — cross-platform Python script that groups includes and ensures the Qt wrapper is applied. Run with `--apply` to write changes.

Basic usage (dry-run):

```
python ./.scripts/include_reflow.py apps/kiosk/src/connection/CheckConnection.cpp
```

Apply changes in-place:

```
python ./.scripts/include_reflow.py --apply apps/kiosk/src/connection/CheckConnection.cpp
```

VS Code integration (recommended)

- Option A (Run on Save extension): install `emeraldwalk.runonsave` and add to `.vscode/settings.json`:

```
"emeraldwalk.runonsave": {
  "commands": [
    {
      "match": "\\.(cpp|cc|cxx|h|hpp)$",
      "cmd": "python ./.scripts/include_reflow.py --apply ${file}"
    }
  ]
}
```

- Option B (Tasks + format on save): create a task that runs the script and wire an editor action to run it on save.

Visual Studio

- Add an external tool entry to run the script and optionally bind a keyboard shortcut.
- For automated formatting on save, use an extension like "Clang Power Tools" or a file watcher to invoke the script.

Qt Creator

- Use the "External" -> "Configure" menu to add a tool that runs the script. Use the "Before Build" step to run it automatically.

Pre-commit hook (enforcement)

- Add `.git/hooks/pre-commit` with the following content (example):

```
#!/usr/bin/env bash
python ./.scripts/include_reflow.py --apply $(git diff --cached --name-only --diff-filter=ACM | grep -E '\\.(cpp|cc|cxx|h|hpp)$' || true)
if ! git diff --quiet; then
  echo "Includes were reformatted. Please review, add and commit changes."
  exit 1
fi
```

Notes

- The script is intentionally conservative: it only rewrites include blocks at the top of files (continuous include block). It will not parse includes appearing in the middle of a file.
- The script is cross-platform and requires Python 3.6+.
