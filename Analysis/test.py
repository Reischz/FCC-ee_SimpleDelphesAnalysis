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
# Assuming HZ4LepLFV_tree is your tree object
all_column_names = HZ4Lep_tree.keys()
print(all_column_names)
# see type of Electron/Electron.Eta
HZ4Lep_df = HZ4Lep_tree.arrays(["Electron"]["Electron.Eta"], library="pd", entry_stop=5)
print(HZ4Lep_df)
print(type(HZ4Lep_df["Electron.Eta"][0]))
print(type(HZ4Lep_df[0]["Electron.Eta"]))