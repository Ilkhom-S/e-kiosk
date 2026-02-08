import os
import re
import glob

def rename_member_vars_in_file(file_path):
    try:
        with open(file_path, 'r') as f:
            content = f.read()

        # Pattern: m[A-Z][a-zA-Z0-9_]* -> m_[A-Z][a-zA-Z0-9_]*
        # This converts mDeviceName to m_DeviceName
        pattern = r'\bm([A-Z][a-zA-Z0-9_]*)\b'
        replacement = r'm_\1'

        new_content = re.sub(pattern, replacement, content)

        if new_content != content:
            with open(file_path, 'w') as f:
                f.write(new_content)
            print(f'Renamed member variables in {file_path}')
            return True
        else:
            print(f'No changes needed in {file_path}')
            return False
    except Exception as e:
        print(f'Error processing {file_path}: {e}')
        return False

# Process all header files in Hardware/Common
hardware_common_headers = glob.glob('include/Hardware/Common/*.h')

print(f"Processing {len(hardware_common_headers)} header files in Hardware/Common...")

changed_files = []
for file_path in hardware_common_headers:
    if rename_member_vars_in_file(file_path):
        changed_files.append(file_path)

# Process all cpp files in Hardware/Common
hardware_common_cpp = glob.glob('src/modules/Hardware/Common/src/*.cpp')

print(f"Processing {len(hardware_common_cpp)} cpp files in Hardware/Common...")

for file_path in hardware_common_cpp:
    if rename_member_vars_in_file(file_path):
        changed_files.append(file_path)

print(f"Hardware/Common refactoring completed. Changed {len(changed_files)} files.")
