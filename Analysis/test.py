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
column_arrays = ["MissingET_size","MissingET_Eta","MissingET_Phi","Jet_size","Electron_size","Electron_Eta","Electron_Phi","Muon_size","Muon_Eta","Muon_Phi"]
HZ4Lep_array = HZ4Lep_tree.arrays(column_arrays, library="pd", entry_stop=1000)
print(type(HZ4Lep_array["Electron_Eta"][0]))