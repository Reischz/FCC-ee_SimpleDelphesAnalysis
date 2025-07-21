import uproot
import matplotlib.pyplot as plt
import numpy as np

infile = uproot.open("output01.root")
intree = infile["Delphes"]

num_muons = intree.arrays("Muon_size", how=list, library="np")

plt.figure(figsize=(10, 8))
plt.hist(num_muons, bins=5, range=(-0.5, 4.5))
plt.title("Number of muons")
plt.xlabel("Number of muons")
plt.ylabel("Event count")
plt.savefig("muon_size.png")

