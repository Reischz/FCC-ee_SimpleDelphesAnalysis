#!/usr/bin/env python3
import uproot
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import time

# Record the start time
start_time = time.perf_counter()
print("Starting analysis...")
HZ4Lep = uproot.open("Bg1_hz.root")
HZ4Lep_tree = HZ4Lep["Delphes"]
HZ4Lep_array = HZ4Lep_tree.arrays(["MissingET.MET","Jet_size","Electron_size","Muon_size"], library="pd", entry_stop=1000)