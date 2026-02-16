#!/usr/bin/env python3
# TODO: To preserve comments in merged INI files, use an external library such as configupdater (https://pypi.org/project/configupdater/) in the future.
"""
merge_ini.py - Section-aware INI merge for EKiosk build system

Usage:
  python3 merge_ini.py --base BASE_INI --platform PLATFORM_INI --output OUTPUT_INI

- BASE_INI: path to base ini template (required)
- PLATFORM_INI: path to platform-specific ini template (optional, can be missing or empty)
- OUTPUT_INI: path to output ini file (required)

Behavior:
- If PLATFORM_INI is missing or empty, just copy BASE_INI to OUTPUT_INI
- If both exist:
  - For each section in PLATFORM_INI:
    - If section not in BASE_INI, add section+keys
    - If section in BASE_INI, platform keys override base keys (platform has precedence)
  - Preserve comments and order as much as possible
- Output is always a valid INI file

Multiplatform: works on Linux, macOS, Windows (Python 3.6+)
"""
import argparse
import os
import sys
import shutil
import configparser


def parse_args():
    parser = argparse.ArgumentParser(description="Section-aware INI merge for EKiosk")
    parser.add_argument('--base', required=True, help='Base ini template path')
    parser.add_argument('--platform', required=False, help='Platform-specific ini template path')
    parser.add_argument('--output', required=True, help='Output ini file path')
    return parser.parse_args()


def merge_ini(base_path, platform_path, output_path):
    # If no platform ini, just copy base
    if not platform_path or not os.path.isfile(platform_path):
        shutil.copyfile(base_path, output_path)
        return

    # Read base and platform
    base = configparser.ConfigParser()
    base.optionxform = str  # preserve case
    base.read(base_path, encoding='utf-8')

    plat = configparser.ConfigParser()
    plat.optionxform = str
    plat.read(platform_path, encoding='utf-8')

    # Merge logic: platform keys override base keys
    for section in plat.sections():
        if not base.has_section(section):
            base.add_section(section)
        for key, value in plat.items(section):
            base.set(section, key, value)  # always override or add

    # Write merged result
    with open(output_path, 'w', encoding='utf-8') as f:
        base.write(f)


def main():
    args = parse_args()
    if not os.path.isfile(args.base):
        print(f"Base ini not found: {args.base}", file=sys.stderr)
        sys.exit(1)
    merge_ini(args.base, args.platform, args.output)

if __name__ == '__main__':
    main()
