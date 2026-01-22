import sys
import os
import subprocess

if len(sys.argv) < 2:
    print("Usage: run_python_cross.py <script> [args...]")
    sys.exit(1)

script = sys.argv[1]
args = sys.argv[2:]

if os.name == 'nt':
    python_cmd = 'python'
else:
    python_cmd = 'python3'

result = subprocess.run([python_cmd, script] + args)
sys.exit(result.returncode)
