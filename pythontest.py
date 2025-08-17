#!/usr/bin/env python
from lib.MadGraphPythiaDelphes_Chain import GenerateSignalChains
import os
import yaml
from datetime import datetime
import subprocess

# os.system("/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC testmadgraph.dat")
testoscomand=subprocess.run(["/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC", "testmadgraph.dat"], capture_output=True, text=True, check=True)
with open('test_output.log', 'w') as f:
    f.write(testoscomand.stdout + '\n' + testoscomand.stderr)

# subprocess.run("cd '20250817_192306_eeToZH_HtoWW_FourLeptons'", shell=True)
run_delphes = subprocess.run(
            ['/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2', 'delphes_card_IDEA.tcl', 'eeToZH_HtoWW_FourLeptons_DelphesOutput.root', 'eeToZH_HtoWW_FourLeptons_pythia8_events.hepmc'],
            capture_output=True, text=True, check=True
        )