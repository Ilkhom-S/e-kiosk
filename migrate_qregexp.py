#!/usr/bin/env python3
import os
import re
import sys

def migrate_qregexp_usage(content):
    """Migrate QRegExp usage to QRegularExpression"""

    # Pattern 1: QRegExp variable declarations and initializations
    # QRegExp rx("pattern") -> QRegularExpression rx("pattern")
    content = re.sub(r'\bQRegExp\s+(\w+)\s*\(\s*([^)]*)\s*\)', r'QRegularExpression \1(\2)', content)

    # Pattern 1b: QRegExp with complex constructor calls
    # QRegExp rx = QRegExp(...) -> QRegularExpression rx = QRegularExpression(...)
    content = re.sub(r'\bQRegExp\s+(\w+)\s*=\s*QRegExp\s*\(\s*([^)]*)\s*\)', r'QRegularExpression \1 = QRegularExpression(\2)', content)

    # Pattern 2: QRegExp array declarations
    # const QRegExp regExps[] = { QRegExp(...), ... } -> const QRegularExpression regExps[] = { QRegularExpression(...), ... }
    content = re.sub(r'\bconst\s+QRegExp\s+(\w+)\[\]', r'const QRegularExpression \1[]', content)

    # Pattern 3: QRegExp in function parameters
    # const QRegExp & -> const QRegularExpression &
    content = re.sub(r'\bconst\s+QRegExp\s*&', r'const QRegularExpression &', content)

    # Pattern 4: QRegExp in QString::remove calls
    # .remove(QRegExp("pattern")) -> .remove(QRegularExpression("pattern"), "")
    content = re.sub(r'\.remove\s*\(\s*QRegExp\s*\(\s*([^)]*)\s*\)\s*\)', r'.remove(QRegularExpression(\1), "")', content)

    # Pattern 5: QRegExp in QStringList::filter calls
    # .filter(QRegExp("pattern")) -> .filter(QRegularExpression("pattern"))
    content = re.sub(r'\.filter\s*\(\s*QRegExp\s*\(\s*([^)]*)\s*\)\s*\)', r'.filter(QRegularExpression(\1))', content)

    # Pattern 6: QRegExp setMinimal calls (deprecated, remove or replace with pattern options)
    content = re.sub(r'(\w+)\.setMinimal\s*\(\s*true\s*\)\s*;', r'//\1.setMinimal(true); // Removed for Qt5/6 compatibility', content)

    # Pattern 7: QRegExp indexIn calls
    # rx.indexIn(text) -> rx.match(text).capturedStart()
    content = re.sub(r'(\w+)\.indexIn\s*\(\s*([^)]*)\s*\)', r'\1.match(\2).capturedStart()', content)

    # Pattern 8: QRegExp indexOf calls (QString method)
    # .indexOf(QRegExp("pattern")) -> .indexOf(QRegularExpression("pattern"))
    content = re.sub(r'\.indexOf\s*\(\s*QRegExp\s*\(\s*([^)]*)\s*\)\s*\)', r'.indexOf(QRegularExpression(\1))', content)

    # Pattern 9: QRegExp lastIndexOf calls (QString method)
    # .lastIndexOf(QRegExp("pattern")) -> .lastIndexOf(QRegularExpression("pattern"))
    content = re.sub(r'\.lastIndexOf\s*\(\s*QRegExp\s*\(\s*([^)]*)\s*\)\s*\)', r'.lastIndexOf(QRegularExpression(\1))', content)

    # Pattern 10: QRegExp contains calls
    # .contains(QRegExp("pattern")) -> .contains(QRegularExpression("pattern"))
    content = re.sub(r'\.contains\s*\(\s*QRegExp\s*\(\s*([^)]*)\s*\)\s*\)', r'.contains(QRegularExpression(\1))', content)

    # Handle exactMatch and cap() patterns more carefully
    # Look for patterns like: if (rx.exactMatch(text)) { ... rx.cap(1) ... }

    def replace_exactmatch_cap(match):
        before = match.group(1)
        var = match.group(2)
        text = match.group(3)
        inside = match.group(4)
        after = match.group(5)

        # Check if there are cap() calls inside
        if '.cap(' in inside:
            # Replace cap() calls
            inside = re.sub(r'\b' + var + r'\.cap\s*\(\s*(\d+)\s*\)', r'match.captured(\1)', inside)
            return f'{before}auto match = {var}.match({text});\n        if (match.hasMatch()) {{\n            {inside}\n        }}{after}'
        else:
            return f'{before}{var}.match({text}).hasMatch(){after}'

    # Pattern for if statements with exactMatch
    content = re.sub(r'(\s+)if\s*\(\s*(\w+)\.exactMatch\s*\(\s*([^)]*)\s*\)\s*\)\s*\{([^}]*)\}(\s*)',
                     replace_exactmatch_cap, content)

    # Handle standalone exactMatch calls
    content = re.sub(r'(\w+)\.exactMatch\s*\(\s*([^)]*)\s*\)', r'\1.match(\2).hasMatch()', content)

    # Handle remaining cap() calls (these might need manual review)
    content = re.sub(r'(\w+)\.cap\s*\(\s*(\d+)\s*\)', r'// TODO: \1.cap(\2) needs manual migration to match.captured(\2)', content)

    return content

def process_file(filepath):
    """Process a single file"""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        original_content = content
        content = migrate_qregexp_usage(content)

        if content != original_content:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False

def main():
    if len(sys.argv) != 2:
        print("Usage: python migrate_qregexp.py <directory>")
        sys.exit(1)

    root_dir = sys.argv[1]

    files_processed = 0
    files_changed = 0

    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if file.endswith(('.cpp', '.h')):
                filepath = os.path.join(root, file)
                files_processed += 1

                if process_file(filepath):
                    files_changed += 1
                    print(f"Updated: {filepath}")

    print(f"\nProcessed {files_processed} files, changed {files_changed} files")

if __name__ == "__main__":
    main()
