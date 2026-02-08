import os
import re
import glob

def rename_member_vars_in_file(file_path):
    try:
        with open(file_path, 'r') as f:
            content = f.read()

        # Pattern to match member variable declarations: type mVariableName;
        # This will match lines like: int mVariableName; or QMutex mMutex;
        # But avoid matching function parameters or other uses
        pattern = r'(\s+)([A-Za-z_][A-Za-z0-9_<>\s\*&]+)\s+(m[A-Z][a-zA-Z0-9_]*)\s*;'

        def replace_match(match):
            indent = match.group(1)
            type_part = match.group(2)
            var_name = match.group(3)
            # Add underscore after m
            new_var_name = 'm_' + var_name[1:]
            return f'{indent}{type_part} {new_var_name};'

        new_content = re.sub(pattern, replace_match, content)

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

# Process all header files in Hardware (excluding Common which is already done)
hardware_headers = []
for root, dirs, files in os.walk('include/Hardware'):
    if 'Common' in root:
        continue  # Skip Common directory
    for file in files:
        if file.endswith('.h'):
            hardware_headers.append(os.path.join(root, file))

print(f"Processing {len(hardware_headers)} header files in Hardware module...")

changed_files = []
for file_path in hardware_headers:
    if rename_member_vars_in_file(file_path):
        changed_files.append(file_path)

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
    if rename_member_vars_in_file(file_path):
        changed_files.append(file_path)

print(f"Hardware module refactoring completed. Changed {len(changed_files)} files.")