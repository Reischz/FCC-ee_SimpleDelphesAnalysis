import uproot
import matplotlib.pyplot as plt
import numpy as np

HZ4Lep = uproot.open("HiggsStrahlungwithFourLeptons.root")
ZWW4Lep = uproot.open("ZWWFourLeptonWithoutTauInLep.root")
HZ4LepLFV = uproot.open("HZtoMuE_LFV.root")
HZ4Lep_tree = HZ4Lep["Delphes"]
ZWW4Lep_tree = ZWW4Lep["Delphes"]
HZ4LepLFV_tree = HZ4LepLFV["Delphes"]

print(type(HZ4Lep_tree))

# num_muons = HZ4Lep_tree.arrays("Muon_size", how=list, library="np")

# plt.figure(figsize=(10, 8))
# plt.hist(num_muons, bins=5, range=(-0.5, 4.5))
# plt.title("Number of muons")
# plt.xlabel("Number of muons")
# plt.ylabel("Event count")
# plt.savefig("muon_size.png")

