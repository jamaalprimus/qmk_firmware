import subprocess
import os

BOARDS = ['redragon/k552/rev1', 'redragon/k552/rev2', 'redragon/k530', 'redragon/k556', 'redragon/k580', 'redragon/k630', 'keychron/rgb/c1', 'keychron/k2', 'keychron/k3', 'keychron/k4/rgb/v1','keychron/k4/rgb/v2', 'keychron/k6/rgb', 'keychron/k6/white', 'keychron/k8', 'ajazz/ak33/rev1', 'ajazz/ak33/rev2', 'smartduck/xs61', 'spcgear/gk530', 'spcgear/gk540', 'sharkoon/sgk3', 'womier/k87', 'flashquark/horizon_z', 'ffc/ffc61', 'gmmk/full/rev2' , 'gmmk/full/rev3' , 'marvo/kg938']

for kb in BOARDS:
    subprocess.run(f"bin/qmk compile -kb {kb} -km all -j{os.cpu_count()}", shell=True)
