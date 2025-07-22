import ROOT
import matplotlib.pyplot as plt

infile = ROOT.TFile("output01.root")
intree = infile.Get("Delphes")

num_muons = []
for i in range(intree.GetEntries()):
	intree.GetEntry(i)
	num_muons.append(intree.Muon_size)

plt.figure(figsize=(10, 8))
plt.hist(num_muons, bins=5, range=(-0.5, 4.5))
plt.title("Number of muons")
plt.xlabel("Number of muons")
plt.ylabel("Event count")
plt.savefig("muon_size.png")

