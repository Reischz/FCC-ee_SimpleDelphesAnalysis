#!/usr/bin/env python3
import uproot
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import time

# Record the start time
start_time = time.perf_counter()
print("Starting analysis...")

def check_jet(tree,name,number=None):
    jet_array = tree.arrays(["Jet_size"], library="pd", entry_stop=number)
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

def check_lepton(tree,name,number=None):
    lep_array = tree.arrays(["Electron_size","Muon_size"], library="pd", entry_stop=number)
    # lep_array_mask = (lep_array["Electron_size"] + lep_array["Muon_size"] >= 2)
    fig=plt.figure()
    ax=fig.add_subplot(111)
    ax.hist(lep_array["Electron_size"] + lep_array["Muon_size"], bins=10, range=(-0.5, 9.5))
    ax.set_title(f"Lepton Size Distribution - {name}")
    ax.set_xlabel("Lepton Size")
    ax.set_ylabel("Count")
    plt.savefig(f"LeptonSize_{name}.png")
    plt.close()
    return 1

def finalstate_fourlepton_cut(name,Earray,Muarray):
    mask = (Earray["Electron_size"] + Muarray["Muon_size"] == 4)
    print(f"For {name}:{np.sum(mask)}/{len(mask)} : {np.sum(mask)/len(mask)*100:.2f}%")
    return mask

# Configuration for testing vs production
TESTING_MODE = True  # Set to False for full analysis
MAX_EVENTS = 1000 if TESTING_MODE else None  # None means read all events
print(f"Running in {'TESTING' if TESTING_MODE else 'PRODUCTION'} mode")
# Load the ROOT files and access the trees
HZ4Lep = uproot.open("Bg1_hz.root")
ZWW4Lep = uproot.open("Bg2_zww.root")
HZ4LepLFV = uproot.open("Sn1_hzLFV.root")
HZ4Lep_tree = HZ4Lep["Delphes"]
ZWW4Lep_tree = ZWW4Lep["Delphes"]
HZ4LepLFV_tree = HZ4LepLFV["Delphes"]

# check_jet(HZ4Lep_tree, "HZ4Lep")
# check_jet(ZWW4Lep_tree, "ZWW4Lep")
# check_jet(HZ4LepLFV_tree, "HZ4LepLFV")
# check_lepton(HZ4Lep_tree, "HZ4Lep")
# check_lepton(ZWW4Lep_tree, "ZWW4Lep")
# check_lepton(HZ4LepLFV_tree, "HZ4LepLFV")
HZ4Lep_array = HZ4Lep_tree.arrays(["Electron_size","Muon_size"], library="pd", entry_stop=MAX_EVENTS)
ZWW4Lep_array = ZWW4Lep_tree.arrays(["Electron_size","Muon_size"], library="pd", entry_stop=MAX_EVENTS)
HZ4LepLFV_array = HZ4LepLFV_tree.arrays(["Electron_size","Muon_size"], library="pd", entry_stop=MAX_EVENTS)
print("Analyzing ratio of Events with exactly 4 leptons...")

finalstate_fourlepton_cut("HZ4Lep", HZ4Lep_array["Electron_size"], HZ4Lep_array["Muon_size"])
finalstate_fourlepton_cut("ZWW4Lep", ZWW4Lep_array["Electron_size"], ZWW4Lep_array["Muon_size"])
finalstate_fourlepton_cut("HZ4LepLFV", HZ4LepLFV_array["Electron_size"], HZ4LepLFV_array["Muon_size"])

# Record the end time
end_time = time.perf_counter()
elapsed_time = end_time - start_time
print(f"Total execution time: {elapsed_time:.2f} seconds")