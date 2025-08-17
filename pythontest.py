#!/usr/bin/env python
from lib.MadGraphPythiaDelphes_Chain import GenerateSignalChains
import os
import yaml
from datetime import datetime
import subprocess

# os.system("/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC testmadgraph.dat")
subprocess.run(["/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC", "testmadgraph.dat"], check=True, capture=True, text=True)
