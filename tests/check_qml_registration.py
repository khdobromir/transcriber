"""Ensure every QtTest QML scenario has a CTest registration."""
import re
import subprocess
import sys

binary, *registered = sys.argv[1:]
result = subprocess.run([binary, "-functions"], capture_output=True, text=True,
                        timeout=20, check=True)
functions = []
for line in result.stdout.splitlines():
    match = re.fullmatch(r"([A-Za-z_][A-Za-z_0-9]*)\(\)", line.strip())
    if not match:
        raise RuntimeError("Unexpected QtTest discovery output: " + line)
    functions.append(match[1])
if not functions or len(set(registered)) != len(registered) or set(functions) != set(registered):
    raise RuntimeError("QML test registration mismatch: missing="
                       + str(sorted(set(functions) - set(registered)))
                       + ", unknown=" + str(sorted(set(registered) - set(functions))))
print("Every QML scenario is registered:", len(functions))
