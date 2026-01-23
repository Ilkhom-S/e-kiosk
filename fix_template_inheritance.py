#!/usr/bin/env python3
import re
import sys

def fix_template_inheritance(content):
    # Keywords to exclude from function call qualification
    keywords = {
        'if', 'for', 'while', 'return', 'switch', 'case', 'default', 'break', 'continue',
        'do', 'else', 'try', 'catch', 'throw', 'new', 'delete', 'sizeof', 'typeof',
        'static_cast', 'dynamic_cast', 'const_cast', 'reinterpret_cast',
        'std', 'QString', 'QByteArray', 'QList', 'QMap', 'QSet', 'QVariant', 'QTimer',
        'QMetaObject', 'Q_ARG', 'SLOT', 'SIGNAL', 'emit',
        'int', 'bool', 'void', 'char', 'float', 'double', 'long', 'short', 'unsigned',
        'const', 'static', 'virtual', 'override', 'final', 'explicit', 'inline',
        'namespace', 'class', 'struct', 'enum', 'union', 'typedef', 'using',
        'public', 'private', 'protected', 'friend',
        'template', 'typename', 'auto', 'lambda', 'mutable'
    }

    lines = content.split('\n')
    fixed_lines = []

    for line in lines:
        original_line = line

        # Skip lines that are comments or already have this->
        if line.strip().startswith('//') or line.strip().startswith('/*') or 'this->' in line:
            fixed_lines.append(line)
            continue

        # Replace unqualified member variables
        # Pattern: space followed by m[A-Z] followed by word chars
        line = re.sub(r'(\s+)(m[A-Z][a-zA-Z0-9_]*)\b', r'\1this->\2', line)

        # NOTE: Function call qualification is too risky - it adds this-> to keywords and macros
        # Only handle member variables for now, function calls need manual review

        fixed_lines.append(line)

    return '\n'.join(fixed_lines)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python fix_template_inheritance.py <file>")
        sys.exit(1)

    filename = sys.argv[1]

    with open(filename, 'r') as f:
        content = f.read()

    fixed_content = fix_template_inheritance(content)

    with open(filename, 'w') as f:
        f.write(fixed_content)

    print(f"Fixed template inheritance in {filename}")
