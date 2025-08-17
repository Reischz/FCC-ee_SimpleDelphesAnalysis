#!/usr/bin/env python
from lib.MadGraphPythiaDelphes_Chain import GenerateSignalChains
import os
import yaml
from datetime import datetime
import subprocess

# i will test all sunction and argument of or.system and subprocess.run
# 1. os.system
testosrun = os.system('/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC testmadgraph.dat')
with open('testosrun.log', 'w') as f:
    f.write(f"Test os.system output: \n{testosrun}\n")

# 2. subprocess.run wo futher arguments
print("=======================================================***======================================================")
print("Testing subprocess.run without further arguments...")
testsubprocessrun = subprocess.run(['/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC', 'testmadgraph.dat'])
with open('testsubprocessrun.log', 'w') as f:
    f.write(f"Test subprocess.run output: \n{testsubprocessrun}\n")

# 3. subprocess.run with capture_output=True
print("=======================================================***======================================================")
print("Testing subprocess.run with capture_output=True...")
testsubprocessrun_capture = subprocess.run(['/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC', 'testmadgraph.dat'], capture_output=True)
with open('testsubprocessrun_capture.log', 'w') as f:
    f.write(f"Test subprocess.run with capture_output=True output: \n{testsubprocessrun_capture.stdout.decode()}\n")

# 4. subprocess.run with text=True
print("=======================================================***======================================================")
print("Testing subprocess.run with text=True...")
testsubprocessrun_text = subprocess.run(['/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC', 'testmadgraph.dat'], text=True)
with open('testsubprocessrun_text.log', 'w') as f:
    f.write(f"Test subprocess.run with text=True output: \n{testsubprocessrun_text.stdout}\n")

# 5. subprocess.run with check=True
print("=======================================================***======================================================")
print("Testing subprocess.run with check=True...")
testsubprocessrun_check = subprocess.run(['/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC', 'testmadgraph.dat'], check=True)
with open('testsubprocessrun_check.log', 'w') as f:
    f.write(f"Test subprocess.run with check=True output: \n{testsubprocessrun_check.stdout}\n")

# 6. subprocess.run with all arguments
print("=======================================================***======================================================")
print("Testing subprocess.run with all arguments...")
testsubprocessrun_all = subprocess.run(['/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC', 'testmadgraph.dat'], capture_output=True, text=True, check=True)
with open('testsubprocessrun_all.log', 'w') as f:
    f.write(f"Test subprocess.run with all arguments output: \n{testsubprocessrun_all.stdout}\n")