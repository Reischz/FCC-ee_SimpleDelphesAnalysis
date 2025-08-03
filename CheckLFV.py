#!/usr/bin/env python3
import uproot
# import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns

HZ4Lep = uproot.open("HiggsStrahlungwithFourLeptons.root")
ZWW4Lep = uproot.open("ZWWFourLeptonWithoutTauInLep.root")
HZ4LepLFV = uproot.open("HZtoMuE_LFV.root")
HZ4Lep_tree = HZ4Lep["Delphes"]
ZWW4Lep_tree = ZWW4Lep["Delphes"]
HZ4LepLFV_tree = HZ4LepLFV["Delphes"]

print(type(HZ4Lep_tree))

HZ4Lep_array = HZ4Lep_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd")
ZWW4Lep_array = ZWW4Lep_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd")
HZ4LepLFV_array = HZ4LepLFV_tree.arrays(["Electron_size","Muon_size","Jet_size"], library="pd")

HZ4Lep_array_mask = (HZ4Lep_array["Muon_size"] + HZ4Lep_array["Electron_size"] == 4) & (HZ4Lep_array["Jet_size"] == 0)
ZWW4Lep_array_mask = (ZWW4Lep_array["Muon_size"] + ZWW4Lep_array["Electron_size"] == 4) & (ZWW4Lep_array["Jet_size"] == 0)
HZ4LepLFV_array_mask = (HZ4LepLFV_array["Muon_size"] + HZ4LepLFV_array["Electron_size"] == 4) & (HZ4LepLFV_array["Jet_size"] == 0)

# Create canvas for three plots
grid = sns.FacetGrid(pd.concat([HZ4Lep_array, ZWW4Lep_array, HZ4LepLFV_array], keys=["HZ4Lep", "ZWW4Lep", "HZ4LepLFV"]),
                     col="key", col_wrap=3, height=5, aspect=1.5)
grid.map_dataframe(sns.histplot, x="Muon_size", bins=5, kde=False, stat="count", discrete=True)
grid.set_axis_labels("Number of Muons", "Event Count") 
grid.set_titles(col_template="{col_name}")
grid.fig.suptitle("Muon Size Distribution for Different Processes", y=1.05)
grid.savefig("muon_size_distribution.png")