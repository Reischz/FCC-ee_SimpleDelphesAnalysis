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