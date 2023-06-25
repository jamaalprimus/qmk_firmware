import subprocess
import os

BOARDS = ['keychron/k2']

for kb in BOARDS:
    subprocess.run(f"bin/qmk compile -kb {kb} -km all -j{os.cpu_count()}", shell=True)
