#!/usr/bin/env python3
import uproot
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import awkward as ak
import seaborn as sns
import os
import vector
import time
# --- CONSTANTS ---
ELECTRON_MASS = 0.000511  # GeV/c^2
MUON_MASS = 0.1057        # GeV/c^2
HIGGS_MASS = 125.0        # GeV/c^2
Z_MASS = 91.1876          # GeV/c^2

def load_data(file_paths, tree_name, columns, max_events):
    """
    Loads data from multiple ROOT files into a dictionary of Akward arrays.
    """
    data = {}
    for name, path in file_paths.items():
        print(f"Loading data from {path}...")
        try:
            with uproot.open(path) as file:
                tree = file[tree_name]
                data[name] = tree.arrays(columns, entry_stop=max_events)
        except Exception as e:
            print(f"Error loading {path}: {e}")
            data[name] = None
    return data

# Configuration
TESTING_MODE = False
MAX_EVENTS = 10000 if TESTING_MODE else None
OUTPUT_DIR = "figure/test"
os.makedirs(OUTPUT_DIR, exist_ok=True)

file_paths = {
    "HZ4Lep": "Bg1_hz.root",
    "ZWW4Lep": "Bg2_zww.root",
    "HZ4LepLFV": "Sn1_hzLFV.root"
}

columns = [
    "MissingET_size", "MissingET.Eta", "MissingET.Phi", "MissingET.MET",
    "Jet_size", "Jet.Phi", "Jet.Eta", "Jet.PT", "Jet.Mass",
    "Electron_size", "Electron.Eta", "Electron.Phi", "Electron.PT","Electron.Charge",
    "Muon_size", "Muon.Eta", "Muon.Phi", "Muon.PT","Muon.Charge"
]

raw_data = load_data(file_paths, "Delphes", columns, MAX_EVENTS)
def plot_three_data(data_arrays, settings, output_dir, exception=False):
    """
    Generates and saves a histogram for three datasets.
    """
    if len(data_arrays) != 3:
        raise ValueError("This function requires exactly three data arrays.")
    
    fig = plt.figure()
    ax = fig.add_subplot(111)
    
    names = list(data_arrays.keys())[::-1]
    
    # Handle both flattened and already flattened arrays
    try:
        n_arr = [ak.flatten(data_arrays[names[2]][settings["name"]]), 
                 ak.flatten(data_arrays[names[1]][settings["name"]]), 
                 ak.flatten(data_arrays[names[0]][settings["name"]])]
    except:
        n_arr = [data_arrays[names[2]], data_arrays[names[1]], data_arrays[names[0]]]
        
    sns.set_theme(style="ticks")
    sns.histplot(n_arr, bins=settings["bins"], binrange=settings["binrange"], ax=ax, fill=False,
                 element=settings["element"], alpha=settings["alpha"], palette=settings['colors'][::-1],
                 stat=settings["stat"], common_norm=False, multiple=settings["multiple"])
    
    ax.set_title(f"{settings['name']} - {settings['prefix']}")
    ax.set_xlabel(settings['xlabel'])
    ax.set_ylabel("Density")
    ax.legend(labels=settings['labels'])
    
    if settings['log_scale']:
        ax.set_yscale("log")

    png_name = f"{settings['prefix']}_{settings['name']}{settings['suffix']}.png"
    plt.savefig(f"{output_dir}/{png_name}")
    print(f"-> {png_name} has been plotted!")
    plt.close()


def plot_data(data_arrays, prefix, output_dir, additional_plot=False):
    """
    Orchestrates the plotting of various variables for the datasets.
    """
    plot_configs = {
        'Electron.PT': {'bins': 65, 'binrange': (0, 130), 'xlabel': "GeV", 'log_scale': False},
        'Jet.PT': {'bins': 65, 'binrange': (0, 130), 'xlabel': "GeV", 'log_scale': False},
        'Jet.Mass': {'bins': 65, 'binrange': (0, 130), 'xlabel': "GeV", 'log_scale': True},
        'Muon.PT': {'bins': 65, 'binrange': (0, 130), 'xlabel': "GeV", 'log_scale': False},
        'Electron.Eta': {'bins': 200, 'binrange': (-10, 10), 'xlabel': "", 'log_scale': False},
        'Muon.Eta': {'bins': 200, 'binrange': (-10, 10), 'xlabel': "", 'log_scale': False},
        'Electron.Phi': {'bins': 140, 'binrange': (-2 * np.pi, 2 * np.pi), 'xlabel': "", 'log_scale': False},
        'Muon.Phi': {'bins': 140, 'binrange': (-2 * np.pi, 2 * np.pi), 'xlabel': "", 'log_scale': False},
        'MissingET.MET': {'bins': 130, 'binrange': (0, 130), 'xlabel': "GeV", 'log_scale': True}
    }

    base_settings = {
        'prefix': prefix, 'element': "step", 'alpha': 0.4,
        'colors': ["#FF9966", "#6698ca", "#66cc99"],
        'labels': ["HZ4LepLFV", "ZWW4Lep", "HZ4Lep"],
        'multiple': "layer", 'stat': "density"
    }

    for name, config in plot_configs.items():
        settings = {**base_settings, **config, 'name': name}
        plot_three_data(data_arrays, settings, output_dir)

def first_cut_4l(array, name):
    """
    Filters events with exactly 4 leptons.
    """
    mask = (array["Electron_size"] + array["Muon_size"] == 4)
    # print(f"    {name}: {np.sum(mask)}/{len(mask)} : {np.sum(mask) / len(mask) * 100:.2f}%")
    return mask

def second_cut_lepton_flavor(array, name, pmask):
    """
    Filters events with an odd number of electrons and muons.
    """
    mask = (array["Electron_size"] % 2 != 0) & (array["Muon_size"] % 2 != 0) & pmask
    # print(f"    {name}: {np.sum(mask)}/{len(mask)} : {np.sum(mask) / len(mask) * 100:.2f}%")
    return mask

print("\nChecking uncut events...")
for name, arr in raw_data.items():
    all_events=MAX_EVENTS if MAX_EVENTS is not None else len(arr)
    # Use a list of dictionaries for easier field addition later
    additional_fields = {
        "3rdCut": [False] * all_events, # Initialize 3rdCut as boolean False
        # "dRThreleptonFromSingleLepton": [[] for _ in range(all_events)],  # Initialize dR with empty lists
        # "lowestdRThreleptonFromSingleLepton": [[] for _ in range(all_events)],  # Initialize lowest dR with empty lists
        # "lowestdRindex": [[] for _ in range(all_events)],  # Initialize lowest dR index with empty lists
        # "InvariantMassFromlowestdR": [[] for _ in range(all_events)],  # Initialize Invariant Mass with empty lists
        # "recoiledZMass": [[] for _ in range(all_events)]  # Initialize recoiled Z Mass with empty lists
        "HiggsCandidateMass": [[] for _ in range(all_events)],  # Initialize Higgs Candidate Mass with empty lists
        "ZCandidateMass": [[] for _ in range(all_events)],  # Initialize Z Candidate Mass with empty lists
        "BestHiggsCandidateMass": [[] for _ in range(all_events)],  # Initialize Best Higgs Candidate Mass with zeros
        "BestZCandidateMass": [[] for _ in range(all_events)],  # Initialize Best Z Candidate Mass with zeros
        "ZvetoHiggsMass": [[] for _ in range(all_events)]  # Initialize Z veto Higgs Mass with empty lists

    }
    print(f"      {name}: {len(arr)} events")
    print(f"-> Applying cut on {name}...")
    # First cut: exactly 4 leptons
    additional_fields["1stCut"] = (arr["Electron_size"] + arr["Muon_size"] == 4)
    # Second cut: odd number leptons
    additional_fields["2ndCut"] = (arr["Electron_size"] % 2 != 0) & (arr["Muon_size"] % 2 != 0) & additional_fields["1stCut"]
    # Post-Second cut: Loop over events that passed the second cut
    pass2cut=[num for num in range(all_events) if additional_fields["2ndCut"][num]]
    for eventnum in pass2cut:
        ElectronSize=arr["Electron_size"][eventnum]
        MuonSize=arr["Muon_size"][eventnum]
        if ElectronSize == 1:
            single_lepton = vector.obj(
                pt=arr["Electron.PT"][eventnum][0], 
                eta=arr["Electron.Eta"][eventnum][0], 
                phi=arr["Electron.Phi"][eventnum][0],
                mass=ELECTRON_MASS
            )
            other_leptons = vector.zip({
                "pt": arr["Muon.PT"][eventnum],
                "eta": arr["Muon.Eta"][eventnum], 
                "phi": arr["Muon.Phi"][eventnum],
                "mass": [MUON_MASS]*3
            })
            single_lepton_charge = arr["Electron.Charge"][eventnum]
            other_leptons_charge = arr["Muon.Charge"][eventnum]
        else:
            single_lepton = vector.obj(
                pt=arr["Muon.PT"][eventnum][0], 
                eta=arr["Muon.Eta"][eventnum][0], 
                phi=arr["Muon.Phi"][eventnum][0],
                mass=MUON_MASS
            )
            other_leptons = vector.zip({
                "pt": arr["Electron.PT"][eventnum],
                "eta": arr["Electron.Eta"][eventnum], 
                "phi": arr["Electron.Phi"][eventnum],
                "mass": [ELECTRON_MASS]*3
            })
            single_lepton_charge = arr["Muon.Charge"][eventnum]
            other_leptons_charge = arr["Electron.Charge"][eventnum]

        # Third cut: non charge-violation
        thirdcut = (single_lepton_charge[0] * ak.prod(other_leptons_charge)) ==1
        if not thirdcut:
            continue

        # Pre-Fourth cut: Invariant mass calculation
        additional_fields["3rdCut"][eventnum] = thirdcut
        pairCandidates = (single_lepton_charge[0] != other_leptons_charge)
        pairFalseIndex=ak.where(pairCandidates==False)[0][0]
        higgs_candidates_mass = []
        z_candidates_mass = []
        for i in range(3):
            higgscanmass = (single_lepton + other_leptons[i]).mass
            zcandidatesmass=(other_leptons[pairFalseIndex] + other_leptons[i]).mass
            higgs_candidates_mass.append(higgscanmass)
            z_candidates_mass.append(zcandidatesmass)

        deltaHiggsMassCandidates = [abs(mass - HIGGS_MASS) for mass in higgs_candidates_mass]
        deltaZMassCandidates = [abs(mass - Z_MASS) for mass in z_candidates_mass]
        nearestHiggsIndex = np.argmin(deltaHiggsMassCandidates)
        secondNearestHiggsIndex = np.argsort(deltaHiggsMassCandidates)[1]
        nearestZIndex = np.argmin(deltaZMassCandidates)
        secondNearestZIndex = np.argsort(deltaZMassCandidates)[1]
        TruthHiggsIndex = nearestHiggsIndex if (nearestHiggsIndex!=pairFalseIndex) else secondNearestHiggsIndex
        TruthZIndex = nearestZIndex if (nearestZIndex!=pairFalseIndex) else secondNearestZIndex
        additional_fields["HiggsCandidateMass"][eventnum] = ak.Array(higgs_candidates_mass)[pairCandidates]
        additional_fields["ZCandidateMass"][eventnum] = ak.Array(z_candidates_mass)[pairCandidates]
        additional_fields["BestHiggsCandidateMass"][eventnum] = [ak.Array(higgs_candidates_mass)[TruthHiggsIndex]]
        additional_fields["BestZCandidateMass"][eventnum] = [ak.Array(z_candidates_mass)[TruthZIndex]]
        pairCandidates=np.array(pairCandidates)
        pairCandidates[TruthZIndex]=False
        ZvetoHiggsIndex= ak.where(pairCandidates==True)[0][0]
        additional_fields["ZvetoHiggsMass"][eventnum] = [ak.Array(higgs_candidates_mass)[ZvetoHiggsIndex]]

        print(f"Processing event {(eventnum+1)/(all_events)*100:.2f}%", end='\r')


    # Add all new fields to the array at once
    for field_name, field_values in additional_fields.items():
        raw_data[name] = ak.with_field(raw_data[name], field_values, field_name)
print("----------------------------***-------------------------------")

plot_configs = {
    # 'dRThreleptonFromSingleLepton': {'bins': 200, 'binrange': (0, 5), 'xlabel': "GeV", 'log_scale': False},
    # 'lowestdRThreleptonFromSingleLepton': {'bins': 200, 'binrange': (0, 5), 'xlabel': "GeV", 'log_scale': False},
    # 'InvariantMassFromlowestdR': {'bins': 400, 'binrange': (0, 200), 'xlabel': "GeV", 'log_scale': False},
    # 'recoiledZMass': {'bins': 400, 'binrange': (0, 200), 'xlabel': "GeV", 'log_scale': False}
    'HiggsCandidateMass': {'bins': 200, 'binrange': (0, 200), 'xlabel': "GeV", 'log_scale': False, 'suffix': ''},
    'ZCandidateMass': {'bins': 200, 'binrange': (0, 200), 'xlabel': "GeV", 'log_scale': False, 'suffix': ''},
    'BestHiggsCandidateMass': {'bins': 200, 'binrange': (0, 200), 'xlabel': "GeV", 'log_scale': False, 'suffix': ''},
    'BestZCandidateMass': {'bins': 200, 'binrange': (0, 200), 'xlabel': "GeV", 'log_scale': False, 'suffix': ''},
    'ZvetoHiggsMass': {'bins': 100, 'binrange': (100, 130), 'xlabel': "GeV", 'log_scale': False, 'suffix': '_zoomed'}
}

base_settings = {
    'prefix': '2ndCut', 'element': "step", 'alpha': 1,
    'colors': ["#FF9966", "#6698ca", "#66cc99"],
    'labels': ["HZ4LepLFV", "ZWW4Lep", "HZ4Lep"],
    'multiple': "layer", 'stat': "density"
}
for name, config in plot_configs.items():
    settings = {**config,**base_settings, 'name': name}
    plot_three_data(raw_data, settings, 'Figure')

# Cut report
print("\nCut report:")
cut_list = ["1stCut", "2ndCut"]
for name, arr in raw_data.items():
    total_events = len(arr["1stCut"])
    print(f"Cut Report for {name}:")
    for cut_name in cut_list:
        mask = arr[cut_name]
        passed_events = np.sum(mask)
        print(f" - {cut_name}: {passed_events}/{total_events} ({passed_events / total_events * 100:.2f}%)")
print("All done!")