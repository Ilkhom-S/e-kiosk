#!/usr/bin/env python3
"""
Script to migrate header files from src/modules/ to include/ according to EKiosk coding standards.
"""

import os
import re
import shutil

def migrate_header(include_path, src_path):
    """Migrate a header file from src/modules/ to include/"""
    print(f"Migrating {include_path} <- {src_path}")

    # Read the source header
    with open(src_path, 'r', encoding='utf-8') as f:
        src_content = f.read()

    # Update the include file with the full class definition
    with open(include_path, 'w', encoding='utf-8') as f:
        f.write(src_content)

    # Update the src file to be a deprecated redirect
    include_rel_path = os.path.relpath(include_path, os.path.dirname(src_path))
    include_rel_path = include_rel_path.replace('\\', '/').replace('../include/', '<').replace('.h', '.h>')

    deprecated_content = f"""/* @file DEPRECATED - See {include_rel_path} instead.

MIGRATION NOTE: This file kept for backward compatibility only.
The class definition has been moved to the public header in {include_rel_path}.
All NEW code should include {include_rel_path}.
*/

#pragma once
#include {include_rel_path}
"""

    with open(src_path, 'w', encoding='utf-8') as f:
        f.write(deprecated_content)

    print(f"✓ Migrated {include_path}")

def main():
    # Find all redirect headers in include/
    include_dir = "/Users/ilkhom/Projects/Humo/e-kiosk/include"

    print(f"Scanning {include_dir} for redirect headers...")

    for root, dirs, files in os.walk(include_dir):
        for file in files:
            if file.endswith('.h'):
                include_path = os.path.join(root, file)
                print(f"Checking {include_path}")

                with open(include_path, 'r', encoding='utf-8') as f:
                    content = f.read()

                # Check if it's a redirect header
                if '#include "../../../src/modules/' in content or '#include "../../../../src/modules/' in content:
                    print(f"Found redirect header: {include_path}")
                    # Extract the src path
                    match = re.search(r'#include "(\.\./\.\./\.\./\.\./src/modules/[^"]+)"', content)
                    if not match:
                        match = re.search(r'#include "(\.\./\.\./\.\./src/modules/[^"]+)"', content)
                    if match:
                        include_stmt = match.group(1)
                        print(f"Include statement: {include_stmt}")
                        # Calculate src_path relative to the include file's directory
                        include_file_dir = os.path.dirname(include_path)
                        src_path = os.path.normpath(os.path.join(include_file_dir, include_stmt))
                        print(f"Calculated src_path: {src_path}")

                        if os.path.exists(src_path):
                            migrate_header(include_path, src_path)
                        else:
                            print(f"Source file not found: {src_path}")
                    else:
                        print(f"No match found in {include_path}")

if __name__ == "__main__":
    main()