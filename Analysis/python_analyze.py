#!/usr/bin/env python3
import uproot
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import time

# Record the start time
start_time = time.time()

def check_jet(tree,name):
    jet_array = tree.arrays(["Jet_size"], library="pd", entry_stop=1000000)
    # jet_array_mask = (jet_array["Jet_size"] >= 2)
    fig=plt.figure()
    ax=fig.add_subplot(111)
    ax.hist(jet_array["Jet_size"], bins=10, range=(-0.5, 9.5))
    ax.set_title(f"Jet Size Distribution - {name}")
    ax.set_xlabel("Jet Size")
    ax.set_ylabel("Count")
    plt.savefig(f"JetSize_{name}.png")
    plt.close()
    return 1

# Configuration for testing vs production
TESTING_MODE = True  # Set to False for full analysis
MAX_EVENTS = 1000 if TESTING_MODE else None  # None means read all events

print(f"Running in {'TESTING' if TESTING_MODE else 'PRODUCTION'} mode")
if TESTING_MODE:
    print(f"Will read only first {MAX_EVENTS} events from each file")

HZ4Lep = uproot.open("Bg1_hz.root")
ZWW4Lep = uproot.open("Bg2_zww.root")
HZ4LepLFV = uproot.open("Sn1_hzLFV.root")
HZ4Lep_tree = HZ4Lep["Delphes"]
ZWW4Lep_tree = ZWW4Lep["Delphes"]
HZ4LepLFV_tree = HZ4LepLFV["Delphes"]

print(type(HZ4Lep_tree))

# Read events (limited for testing or all for production)
if TESTING_MODE:
    HZ4Lep_array = HZ4Lep_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd", entry_stop=MAX_EVENTS)
    ZWW4Lep_array = ZWW4Lep_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd", entry_stop=MAX_EVENTS)
    HZ4LepLFV_array = HZ4LepLFV_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd", entry_stop=MAX_EVENTS)
else:
    HZ4Lep_array = HZ4Lep_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd")
    ZWW4Lep_array = ZWW4Lep_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd")
    HZ4LepLFV_array = HZ4LepLFV_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd")

HZ4Lep_array_mask = (HZ4Lep_array["Muon_size"] + HZ4Lep_array["Electron_size"] == 4)
ZWW4Lep_array_mask = (ZWW4Lep_array["Muon_size"] + ZWW4Lep_array["Electron_size"] == 4)
HZ4LepLFV_array_mask = (HZ4LepLFV_array["Muon_size"] + HZ4LepLFV_array["Electron_size"] == 4)

# passed condition events count
print(f"HZ4Lep events count: {HZ4Lep_array_mask.sum()}")
print(f"ZWW4Lep events count: {ZWW4Lep_array_mask.sum()}")
print(f"HZ4LepLFV events count: {HZ4LepLFV_array_mask.sum()}")
# Create canvas for three plots: 2 in first row, 1 centered in second row
fig = plt.figure(figsize=(12, 10))

# First row - two plots
ax1 = plt.subplot(2, 2, 1)  # Top left
ax2 = plt.subplot(2, 2, 2)  # Top right
# Second row - one centered plot using subplot2grid to center it
ax3 = plt.subplot2grid((2, 4), (1, 1), colspan=2)  # Bottom center, spanning 2 columns

# Plot HZ4Lep
data_hz4lep = HZ4Lep_array[HZ4Lep_array_mask]["Muon_size"]
ax1.hist(data_hz4lep, bins=5, range=(-0.5, 4.5), alpha=0.7, edgecolor='black')
ax1.set_title("HZ4Lep")
ax1.set_xlabel("Muon Size")
ax1.set_ylabel("Count")
# Add stats text
stats_text = f"Count: {len(data_hz4lep)}\nMean: {data_hz4lep.mean():.2f}\nStd: {data_hz4lep.std():.2f}"
ax1.text(0.02, 0.98, stats_text, transform=ax1.transAxes, verticalalignment='top', 
         bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

# Plot ZWW4Lep
data_zww4lep = ZWW4Lep_array[ZWW4Lep_array_mask]["Muon_size"]
ax2.hist(data_zww4lep, bins=5, range=(-0.5, 4.5), alpha=0.7, edgecolor='black')
ax2.set_title("ZWW4Lep")
ax2.set_xlabel("Muon Size")
ax2.set_ylabel("Count")
# Add stats text
stats_text = f"Count: {len(data_zww4lep)}\nMean: {data_zww4lep.mean():.2f}\nStd: {data_zww4lep.std():.2f}"
ax2.text(0.02, 0.98, stats_text, transform=ax2.transAxes, verticalalignment='top',
         bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

# Plot HZ4LepLFV
data_hz4leplfv = HZ4LepLFV_array[HZ4LepLFV_array_mask]["Muon_size"]
ax3.hist(data_hz4leplfv, bins=5, range=(-0.5, 4.5), alpha=0.7, edgecolor='black')
ax3.set_title("HZ4LepLFV")
ax3.set_xlabel("Muon Size")
ax3.set_ylabel("Count")
# Add stats text
stats_text = f"Count: {len(data_hz4leplfv)}\nMean: {data_hz4leplfv.mean():.2f}\nStd: {data_hz4leplfv.std():.2f}"
ax3.text(0.02, 0.98, stats_text, transform=ax3.transAxes, verticalalignment='top',
         bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
# Add overall title
fig.suptitle("Muon Size Distribution in Different Datasets")
# Save to PNG
plt.savefig("CheckLFV_Muon.png")

# Close the plot to free memory
plt.close()
# Plot Electron Size Distribution
fig = plt.figure(figsize=(12, 10))

# First row - two plots
ax1 = plt.subplot(2, 2, 1)  # Top left
ax2 = plt.subplot(2, 2, 2)  # Top right
# Second row - one centered plot using subplot2grid to center it
ax3 = plt.subplot2grid((2, 4), (1, 1), colspan=2)  # Bottom center, spanning 2 columns

# Plot HZ4Lep
data_hz4lep_e = HZ4Lep_array[HZ4Lep_array_mask]["Electron_size"]
ax1.hist(data_hz4lep_e, bins=5, range=(-0.5, 4.5), alpha=0.7, edgecolor='black')
ax1.set_title("HZ4Lep")
ax1.set_xlabel("Electron Size")
ax1.set_ylabel("Count")
# Add stats text
stats_text = f"Count: {len(data_hz4lep_e)}\nMean: {data_hz4lep_e.mean():.2f}\nStd: {data_hz4lep_e.std():.2f}"
ax1.text(0.02, 0.98, stats_text, transform=ax1.transAxes, verticalalignment='top', 
         bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

# Plot ZWW4Lep
data_zww4lep_e = ZWW4Lep_array[ZWW4Lep_array_mask]["Electron_size"]
ax2.hist(data_zww4lep_e, bins=5, range=(-0.5, 4.5), alpha=0.7, edgecolor='black')
ax2.set_title("ZWW4Lep")
ax2.set_xlabel("Electron Size")
ax2.set_ylabel("Count")
# Add stats text
stats_text = f"Count: {len(data_zww4lep_e)}\nMean: {data_zww4lep_e.mean():.2f}\nStd: {data_zww4lep_e.std():.2f}"
ax2.text(0.02, 0.98, stats_text, transform=ax2.transAxes, verticalalignment='top',
         bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

# Plot HZ4LepLFV
data_hz4leplfv_e = HZ4LepLFV_array[HZ4LepLFV_array_mask]["Electron_size"]
ax3.hist(data_hz4leplfv_e, bins=5, range=(-0.5, 4.5), alpha=0.7, edgecolor='black')
ax3.set_title("HZ4LepLFV")
ax3.set_xlabel("Electron Size")
ax3.set_ylabel("Count")
# Add stats text
stats_text = f"Count: {len(data_hz4leplfv_e)}\nMean: {data_hz4leplfv_e.mean():.2f}\nStd: {data_hz4leplfv_e.std():.2f}"
ax3.text(0.02, 0.98, stats_text, transform=ax3.transAxes, verticalalignment='top',
         bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
# Add overall title
fig.suptitle("Electron Size Distribution in Different Datasets")
# Save to PNG
plt.savefig("CheckLFV_Electron.png")

# 3. Record the end time
end_time = time.time()

# 4. Calculate the elapsed time
elapsed_time = end_time - start_time
check_jet(HZ4Lep_tree, "HZ4Lep")
check_jet(ZWW4Lep_tree, "ZWW4Lep")
check_jet(HZ4LepLFV_tree, "HZ4LepLFV")

print(f"Total execution time: {elapsed_time:.2f} seconds")