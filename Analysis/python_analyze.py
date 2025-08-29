#!/usr/bin/env python3
import uproot
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import time

# Record the start time
start_time = time.perf_counter()
print("Starting analysis...")

def check_jet(array,name,mask):
    jet_array = array[mask]
    # jet_array_mask = (jet_array["Jet_size"] >= 2)
    fig=plt.figure()
    ax=fig.add_subplot(111)
    ax.hist(jet_array, bins=10, range=(-0.5, 9.5))
    ax.set_title(f"Jet Size Distribution - {name}")
    ax.set_xlabel("Jet Size")
    ax.set_ylabel("Count")
    plt.savefig(f"JetSize_{name}.png")
    plt.close()
    return 1

def check_lepton(Earray, Muarray, name, mask):
    fig=plt.figure()
    ax=fig.add_subplot(111)
    ax.hist(Earray[mask] + Muarray[mask], bins=10, range=(-0.5, 9.5))
    ax.set_title(f"Lepton Size Distribution - {name}")
    ax.set_xlabel("Lepton Size")
    ax.set_ylabel("Count")
    plt.savefig(f"LeptonSize_{name}.png")
    plt.close()
    return 1

def finalstate_fourlepton_cut(name,Earray,Muarray):
    mask = (Earray + Muarray == 4)
    print(f"For {name}:{np.sum(mask)}/{len(mask)} : {np.sum(mask)/len(mask)*100:.2f}%")
    return mask

def check_drFromMET(array,name,mask):
    # Placeholder for future implementation
    fig=plt.figure()
    ax=fig.add_subplot(111)
    ax.hist(array[mask], bins=10, range=(-0.5, 9.5))
    ax.set_title("MET_size Distribution")
    ax.set_xlabel("MET_size")
    ax.set_ylabel("Count")
    plt.savefig(f"MET_size_{name}.png")
    plt.close()
    return 1


# Configuration for testing vs production
TESTING_MODE = False  # Set to False for full analysis
MAX_EVENTS = 1000 if TESTING_MODE else None  # None means read all events
print(f"Running in {'TESTING' if TESTING_MODE else 'PRODUCTION'} mode")
# Load the ROOT files and access the trees
HZ4Lep = uproot.open("Bg1_hz.root")
ZWW4Lep = uproot.open("Bg2_zww.root")
HZ4LepLFV = uproot.open("Sn1_hzLFV.root")
HZ4Lep_tree = HZ4Lep["Delphes"]
ZWW4Lep_tree = ZWW4Lep["Delphes"]
HZ4LepLFV_tree = HZ4LepLFV["Delphes"]

# Check for events with exactly 4 leptons
column_arrays = ["MissingET_size","Jet_size","Electron_size","Muon_size"]
HZ4Lep_array = HZ4Lep_tree.arrays(column_arrays, library="pd", entry_stop=MAX_EVENTS)
ZWW4Lep_array = ZWW4Lep_tree.arrays(column_arrays, library="pd", entry_stop=MAX_EVENTS)
HZ4LepLFV_array = HZ4LepLFV_tree.arrays(column_arrays, library="pd", entry_stop=MAX_EVENTS)
print("Analyzing ratio of Events with exactly 4 leptons...")
HZ4Lep_4lcut=finalstate_fourlepton_cut("HZ4Lep", HZ4Lep_array["Electron_size"], HZ4Lep_array["Muon_size"])
ZWW4Lep_4lcut=finalstate_fourlepton_cut("ZWW4Lep", ZWW4Lep_array["Electron_size"], ZWW4Lep_array["Muon_size"])
HZ4LepLFV_4lcut=finalstate_fourlepton_cut("HZ4LepLFV", HZ4LepLFV_array["Electron_size"], HZ4LepLFV_array["Muon_size"])

# Check jet distributions
check_jet(HZ4Lep_array["Jet_size"], "HZ4Lep", np.ones(len(HZ4Lep_array), dtype=bool))
check_jet(ZWW4Lep_array["Jet_size"], "ZWW4Lep", np.ones(len(ZWW4Lep_array), dtype=bool))
check_jet(HZ4LepLFV_array["Jet_size"], "HZ4LepLFV", np.ones(len(HZ4LepLFV_array), dtype=bool))
# Check jet and lepton distributions again after 4-lepton cut
check_jet(HZ4Lep_array["Jet_size"], "4lCut_HZ4Lep", HZ4Lep_4lcut)
check_jet(ZWW4Lep_array["Jet_size"], "4lCut_ZWW4Lep", ZWW4Lep_4lcut)
check_jet(HZ4LepLFV_array["Jet_size"], "4lCut_HZ4LepLFV", HZ4LepLFV_4lcut)

# check MET distributions
check_drFromMET(HZ4Lep_array["MissingET_size"], "HZ4Lep", np.ones(len(HZ4Lep_array), dtype=bool))
check_drFromMET(ZWW4Lep_array["MissingET_size"], "ZWW4Lep", np.ones(len(ZWW4Lep_array), dtype=bool))
check_drFromMET(HZ4LepLFV_array["MissingET_size"], "HZ4LepLFV", np.ones(len(HZ4LepLFV_array), dtype=bool))
# Record the end time
end_time = time.perf_counter()
elapsed_time = end_time - start_time
print(f"Total execution time: {elapsed_time:.2f} seconds")