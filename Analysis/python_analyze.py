#!/usr/bin/env python3
import uproot
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import time

# Record the start time
start_time = time.perf_counter()
print("Starting analysis...")

def check_jet(jet_array,name):
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
    print(f"    {name}:{np.sum(mask)}/{len(mask)} : {np.sum(mask)/len(mask)*100:.2f}%")
    return mask

def check_MET_distribution(array,name):
    fig=plt.figure()
    ax=fig.add_subplot(111)
    ax.hist(array, bins=10, range=(-0.5, 9.5))
    ax.set_title(f"MET_size Distribution - {name}")
    ax.set_xlabel("MET_size")
    ax.set_ylabel("Count")
    plt.savefig(f"MET_size_{name}.png")
    plt.close()
    return 1

def check_drFromMET(n_array, name):
    # Calculate delta R between MET and each lepton
    fig=plt.figure()
    ax=fig.add_subplot(111)
    ax.set_title(f"Delta R from MET to Leptons - {name}")
    ax.set_xlabel("Delta R")
    ax.set_ylabel("Count")
    deta_r=np.array([])
    for number_event in range(len(n_array["MissingET_size"])):
        enum=n_array["Electron_size"][number_event]
        for ie in range(enum):
            dphi=n_array["Electron.Phi"][number_event][ie]-n_array["MissingET.Phi"][number_event]
            while (dphi > np.pi):
                dphi -= 2*np.pi
            while (dphi < -np.pi):
                dphi += 2*np.pi
            dr=np.sqrt((n_array["Electron.Eta"][number_event][ie]-n_array["MissingET.Eta"][number_event])**2 + dphi**2)
            deta_r=np.append(deta_r, dr)
        mnum=n_array["Muon_size"][number_event]
        for im in range(mnum):
            dphi=n_array["Muon.Phi"][number_event][im]-n_array["MissingET.Phi"][number_event]
            while (dphi > np.pi):
                dphi -= 2*np.pi
            while (dphi < -np.pi):
                dphi += 2*np.pi
            dr=np.sqrt((n_array["Muon.Eta"][number_event][im]-n_array["MissingET.Eta"][number_event])**2 + dphi**2)
            deta_r=np.append(deta_r, dr) 
            
    ax.hist(deta_r, bins=100, range=(0, 5))

    plt.savefig(f"DeltaR_MET_Leptons_{name}.png")
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
column_arrays = ["MissingET_size","MissingET.Eta","MissingET.Phi","Jet_size","Electron_size","Electron.Eta","Electron.Phi","Muon_size","Muon.Eta","Muon.Phi"]
HZ4Lep_array = HZ4Lep_tree.arrays(column_arrays, library="np", entry_stop=MAX_EVENTS)
ZWW4Lep_array = ZWW4Lep_tree.arrays(column_arrays, library="np", entry_stop=MAX_EVENTS)
HZ4LepLFV_array = HZ4LepLFV_tree.arrays(column_arrays, library="np", entry_stop=MAX_EVENTS)
print("Analyzing ratio of Events with exactly 4 leptons...")
HZ4Lep_4lcut=finalstate_fourlepton_cut("HZ4Lep", HZ4Lep_array["Electron_size"], HZ4Lep_array["Muon_size"])
ZWW4Lep_4lcut=finalstate_fourlepton_cut("ZWW4Lep", ZWW4Lep_array["Electron_size"], ZWW4Lep_array["Muon_size"])
HZ4LepLFV_4lcut=finalstate_fourlepton_cut("HZ4LepLFV", HZ4LepLFV_array["Electron_size"], HZ4LepLFV_array["Muon_size"])

# Check jet distributions
check_jet(HZ4Lep_array["Jet_size"], "HZ4Lep")
check_jet(ZWW4Lep_array["Jet_size"], "ZWW4Lep")
check_jet(HZ4LepLFV_array["Jet_size"], "HZ4LepLFV")
# Check jet and lepton distributions again after 4-lepton cut
check_jet(HZ4Lep_array["Jet_size"][HZ4Lep_4lcut], "4lCut_HZ4Lep")
check_jet(ZWW4Lep_array["Jet_size"][ZWW4Lep_4lcut], "4lCut_ZWW4Lep")
check_jet(HZ4LepLFV_array["Jet_size"][HZ4LepLFV_4lcut], "4lCut_HZ4LepLFV")

# check MET distributions
check_MET_distribution(HZ4Lep_array, "HZ4Lep")
check_MET_distribution(ZWW4Lep_array, "ZWW4Lep")
check_MET_distribution(HZ4LepLFV_array, "HZ4LepLFV")

# check dr from MET to all lepton
check_drFromMET(HZ4Lep_array, "HZ4Lep")
check_drFromMET(ZWW4Lep_array, "ZWW4Lep")
check_drFromMET(HZ4LepLFV_array, "HZ4LepLFV")

# Record the end time
end_time = time.perf_counter()
elapsed_time = end_time - start_time
print(f"Total execution time: {elapsed_time:.2f} seconds")