#!/usr/bin/env python3

import os
import glob

# Directory containing the src headers
src_dir = "src/modules/Hardware/Common/src"
include_dir = "include/Hardware/Common"

# Find all .h files in src directory (recursively)
header_files = glob.glob(os.path.join(src_dir, "**", "*.h"), recursive=True)

converted_count = 0
for header_file in header_files:
    # Get the relative path from src/modules/Hardware/Common/src
    rel_path = os.path.relpath(header_file, src_dir)

    # Check if the corresponding include file exists
    include_file = os.path.join(include_dir, rel_path)
    if os.path.exists(include_file):
        # The include path should be Hardware/Common/ + rel_path (which is already the relative path from include/Hardware/Common/)
        include_path = "Hardware/Common/" + rel_path

        # Create the redirect content
        content = f"""/* @file DEPRECATED - See include/{include_path} instead.

MIGRATION NOTE: This file kept for backward compatibility only.
The class definition has been moved to the public header in include/.
All NEW code should include <{include_path}>.
*/

#pragma once
#include <{include_path}>
"""

        # Write the redirect header
        with open(header_file, 'w', encoding='utf-8') as f:
            f.write(content)

        print(f"Converted {header_file} to redirect header (include: {include_path})")
        converted_count += 1
    else:
        print(f"Skipped {header_file} - no corresponding include file found")

print(f"Converted {converted_count} header files to redirect headers out of {len(header_files)} total")
