import os
import re
import glob

def rename_member_vars(file_path):
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
        else:
            print(f'No changes needed in {file_path}')
    except Exception as e:
        print(f'Error processing {file_path}: {e}')

# Process all header files in Hardware (excluding Common which is already done)
hardware_headers = []
for root, dirs, files in os.walk('include/Hardware'):
    if 'Common' in root:
        continue  # Skip Common directory
    for file in files:
        if file.endswith('.h'):
            hardware_headers.append(os.path.join(root, file))

print(f"Processing {len(hardware_headers)} header files in Hardware module...")

for file_path in hardware_headers:
    rename_member_vars(file_path)

# Process all cpp files in Hardware (excluding Common)
hardware_cpp = []
for root, dirs, files in os.walk('src/modules/Hardware'):
    if 'Common' in root:
        continue  # Skip Common directory
    for file in files:
        if file.endswith('.cpp'):
            hardware_cpp.append(os.path.join(root, file))

print(f"Processing {len(hardware_cpp)} cpp files in Hardware module...")

for file_path in hardware_cpp:
    rename_member_vars(file_path)

print("Hardware module refactoring completed")
