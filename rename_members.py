#!/usr/bin/env python3
"""
Script to rename member variables from mCamelCase to m_camelCase
across the entire codebase.

Usage:
    python rename_members.py [--dry-run] [--files FILE1 FILE2 ...]

Options:
    --dry-run    Show what would be changed without making changes
    --files      Specific files to process (default: all .h and .cpp files)
"""

import os
import re
import argparse
import subprocess
from pathlib import Path
from typing import Dict, List, Tuple, Set

class MemberRenamer:
    def __init__(self, dry_run: bool = False):
        self.dry_run = dry_run
        self.changes_made = 0
        self.files_processed = 0

    def camel_to_snake(self, name: str) -> str:
        """Convert mCamelCase to m_camelCase (Qt convention)"""
        if not name.startswith('m'):
            return name

        # Remove 'm' prefix temporarily
        without_m = name[1:]

        # For Qt convention: m_camelCase means m_ + camelCase with first letter lowercase
        # So mDeviceList becomes m_deviceList
        if without_m:
            # Lowercase the first letter of the camelCase part
            camel_case = without_m[0].lower() + without_m[1:]
            return f'm_{camel_case}'
        else:
            return 'm_'

    def find_member_variables(self, content: str) -> Set[str]:
        """Find all member variables in private/protected sections"""
        variables = set()

        # Split content into sections based on access specifiers
        sections = re.split(r'\b(private|protected|public)\s*:', content)

        for i, section in enumerate(sections):
            if section in ['private', 'protected'] and i + 1 < len(sections):
                # Extract member variables from this section
                section_content = sections[i + 1]

                # Find variable declarations (simple pattern)
                # Look for lines that start with member variable names
                lines = section_content.split('\n')
                for line in lines:
                    line = line.strip()
                    if not line or line.startswith('//') or line.startswith('/*'):
                        continue

                    # Match member variables (mSomething)
                    matches = re.findall(r'\bm[A-Z][a-zA-Z0-9_]*\b', line)
                    for match in matches:
                        # Check if it's a variable declaration (not a function call, etc.)
                        if not re.search(rf'{re.escape(match)}\s*\(', line):  # Not a function
                            variables.add(match)

        return variables

    def process_file(self, file_path: Path) -> bool:
        """Process a single file"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()

            variables = self.find_member_variables(content)
            if not variables:
                return False

            modified_content = content
            changes = []

            for var in sorted(variables):
                new_var = self.camel_to_snake(var)
                if new_var != var:
                    # Replace all occurrences of the variable
                    modified_content = re.sub(rf'\b{re.escape(var)}\b', new_var, modified_content)
                    changes.append(f'  {var} -> {new_var}')

            if changes and modified_content != content:
                if self.dry_run:
                    print(f"Would update {file_path}:")
                    for change in changes:
                        print(change)
                else:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(modified_content)
                    print(f"Updated {file_path}")

                self.changes_made += len(changes)
                return True

        except Exception as e:
            print(f"Error processing {file_path}: {e}")

        return False

    def process_file_pair(self, header_file: Path, impl_file: Path = None) -> bool:
        """Process a header/implementation file pair"""
        header_changed = self.process_file(header_file)
        impl_changed = False

        if impl_file and impl_file.exists():
            impl_changed = self.process_file(impl_file)

        return header_changed or impl_changed

    def process_files(self, files: List[Path]) -> None:
        """Process all files, grouping by base name"""
        file_groups = {}

        for file_path in files:
            if file_path.suffix in ['.h', '.hpp']:
                base_name = file_path.stem
                if base_name not in file_groups:
                    file_groups[base_name] = {}
                file_groups[base_name]['header'] = file_path
            elif file_path.suffix in ['.cpp', '.cc', '.cxx']:
                base_name = file_path.stem
                if base_name not in file_groups:
                    file_groups[base_name] = {}
                file_groups[base_name]['impl'] = file_path

        print(f"Found {len(files)} files in {len(file_groups)} groups to process")

        for base_name, file_dict in file_groups.items():
            header_file = file_dict.get('header')
            impl_file = file_dict.get('impl')

            if self.process_file_pair(header_file, impl_file):
                self.files_processed += 1

        print(f"\nSummary:")
        print(f"Files processed: {self.files_processed}")
        print(f"Files changed: {self.files_processed}")

def main():
    parser = argparse.ArgumentParser(description="Rename member variables from mCamelCase to m_camelCase")
    parser.add_argument('--dry-run', action='store_true', help="Show what would be changed without making changes")
    parser.add_argument('--files', nargs='*', help="Specific files to process")
    parser.add_argument('--root', default='.', help="Root directory to process")

    args = parser.parse_args()

    renamer = MemberRenamer(dry_run=args.dry_run)

    if args.files:
        files = [Path(f) for f in args.files if f.endswith(('.h', '.hpp', '.cpp', '.cc', '.cxx'))]
    else:
        root_path = Path(args.root)
        files = []
        for ext in ['.h', '.hpp', '.cpp', '.cc', '.cxx']:
            files.extend(root_path.rglob(f'*{ext}'))

    renamer.process_files(files)

if __name__ == '__main__':
    main()