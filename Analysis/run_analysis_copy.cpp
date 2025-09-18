#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1D.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "classes/DelphesClasses.h" // Include Delphes classes header

// --- Physics Constants ---
const double ELECTRON_MASS = 0.000511; // GeV/c^2
const double MUON_MASS = 0.1057;       // GeV/c^2
const double HIGGS_MASS = 125.0;       // GeV/c^2
const double Z_MASS = 91.1876;         // GeV/c^2
const double BEAM_ENERGY = 240.0;      // GeV (for recoil mass)

// --- Helper Struct for Histograms ---
struct Histograms {
    TH1D* Z_selectionMass;
    TH1D* Higgs_recoiledMass;
    TH1D* Higgs_reconstructedMass;
    TH1D* Higgs_selectionMass;
    TH1D* Z_recoiledMass;
    TH1D* Z_reconstructedMass;
    
    long long n_total = 0, n_pass_cut1 = 0, n_pass_cut2 = 0, n_pass_cut3 = 0;

    Histograms(const std::string& prefix) {
        Z_selectionMass         = new TH1D((prefix + "_Z_selectionMass").c_str(), "Z Mass (from Z selection);GeV;Entries", 400, 50, 150);
        Higgs_recoiledMass      = new TH1D((prefix + "_Higgs_recoiledMass").c_str(), "Higgs Recoil Mass (from Z selection);GeV;Entries", 400, 100, 150);
        Higgs_reconstructedMass = new TH1D((prefix + "_Higgs_reconstructedMass").c_str(), "Higgs Reconstructed Mass (from Z selection);GeV;Entries", 400, 100, 150);
        Higgs_selectionMass     = new TH1D((prefix + "_Higgs_selectionMass").c_str(), "Higgs Mass (from Higgs selection);GeV;Entries", 400, 100, 150);
        Z_recoiledMass          = new TH1D((prefix + "_Z_recoiledMass").c_str(), "Z Recoil Mass (from Higgs selection);GeV;Entries", 400, 50, 150);
        Z_reconstructedMass     = new TH1D((prefix + "_Z_reconstructedMass").c_str(), "Z Reconstructed Mass (from Higgs selection);GeV;Entries", 400, 50, 150);
    }
};

// --- Main Analysis Function ---
void run_analysis_copy() {
    gROOT->SetBatch(kTRUE); // Run in batch mode

    // --- Configuration ---
    std::map<std::string, std::string> file_paths = {
        {"HZ4Lep", "Bg1_hz.root"},
        {"ZWW4Lep", "Bg2_zww.root"},
        {"HZ4LepLFV", "Sn1_hzLFV.root"}
    };
    
    std::map<std::string, Histograms> hist_map;
    for (auto const& [name, path] : file_paths) {
        hist_map.emplace(name, Histograms(name));
    }
    
    TLorentzVector beam_vector(0, 0, 0, BEAM_ENERGY);

    // Loop over each DATASET
    for (auto const& [name, path] : file_paths) {
        TFile* file = TFile::Open(path.c_str());
        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << path << std::endl;
            continue;
        }
        TTree* tree = (TTree*)file->Get("Delphes");
        if (!tree) {
            std::cerr << "Error: TTree 'Delphes' not found in " << path << std::endl;
            file->Close();
            delete file;
            continue;
        }
        
        std::cout << "Processing file: " << path << " for dataset: " << name << std::endl;

        // --- Manual Branch Addressing ---
        TClonesArray *branchElectron = nullptr;
        TClonesArray *branchMuon = nullptr;
        tree->SetBranchAddress("Electron", &branchElectron);
        tree->SetBranchAddress("Muon", &branchMuon);

        long long n_entries = tree->GetEntries();
        hist_map.at(name).n_total = n_entries;

        // --- Event Loop ---
        for (long long i = 0; i < n_entries; ++i) {
            tree->GetEntry(i);

            int electron_size = branchElectron->GetEntries();
            int muon_size = branchMuon->GetEntries();

            // --- Cut 1: Exactly 4 leptons ---
            if (electron_size + muon_size != 4) continue;
            hist_map.at(name).n_pass_cut1++;
            
            // --- Cut 2: Odd number of electrons and muons ---
            if (electron_size % 2 == 0 || muon_size % 2 == 0) continue;
            hist_map.at(name).n_pass_cut2++;

            // --- Particle Reconstruction ---
            std::vector<TLorentzVector> electrons, muons;
            int total_charge = 0;
            int charge_product = 1;

            for (int j = 0; j < electron_size; ++j) {
                Electron *e = (Electron*) branchElectron->At(j);
                TLorentzVector p;
                p.SetPtEtaPhiM(e->PT, e->Eta, e->Phi, ELECTRON_MASS);
                electrons.push_back(p);
                total_charge += e->Charge;
                charge_product *= e->Charge;
            }
            for (int j = 0; j < muon_size; ++j) {
                Muon *m = (Muon*) branchMuon->At(j);
                TLorentzVector p;
                p.SetPtEtaPhiM(m->PT, m->Eta, m->Phi, MUON_MASS);
                muons.push_back(p);
                total_charge += m->Charge;
                charge_product *= m->Charge;
            }

            // --- Cut 3: Charge conservation ---
            if (total_charge != 0 || charge_product != 1) continue;
            hist_map.at(name).n_pass_cut3++;

            // Separate into LFV candidate types
            std::vector<TLorentzVector> other_leptons, single_lepton_type;
            if (electron_size == 1) {
                single_lepton_type = electrons;
                other_leptons = muons;
            } else {
                single_lepton_type = muons;
                other_leptons = electrons;
            }

            // === Strategy 1: Find Z -> l+l- ===
            double best_Z_mass_diff = 1e9;
            TLorentzVector best_Z_candidate;
            int z_idx1 = -1, z_idx2 = -1;

            if (other_leptons.size() == 3) {
                 for (int j = 0; j < 3; ++j) {
                    for (int k = j + 1; k < 3; ++k) {
                        double mass = (other_leptons[j] + other_leptons[k]).M();
                        if (abs(mass - Z_MASS) < best_Z_mass_diff) {
                            best_Z_mass_diff = abs(mass - Z_MASS);
                            best_Z_candidate = other_leptons[j] + other_leptons[k];
                            z_idx1 = j;
                            z_idx2 = k;
                        }
                    }
                }
            }
            
            if (z_idx1 != -1) {
                hist_map.at(name).Z_selectionMass->Fill(best_Z_candidate.M());
                hist_map.at(name).Higgs_recoiledMass->Fill((beam_vector - best_Z_candidate).M());
                int lfv_idx = 3 - z_idx1 - z_idx2;
                TLorentzVector higgs_reconstructed = single_lepton_type[0] + other_leptons[lfv_idx];
                hist_map.at(name).Higgs_reconstructedMass->Fill(higgs_reconstructed.M());
            }

            // === Strategy 2: Find H -> e mu ===
            double best_H_mass_diff = 1e9;
            TLorentzVector best_H_candidate;
            int h_electron_idx = -1, h_muon_idx = -1;

            for (size_t j = 0; j < electrons.size(); ++j) {
                for (size_t k = 0; k < muons.size(); ++k) {
                     double mass = (electrons[j] + muons[k]).M();
                     if (abs(mass - HIGGS_MASS) < best_H_mass_diff) {
                         best_H_mass_diff = abs(mass - HIGGS_MASS);
                         best_H_candidate = electrons[j] + muons[k];
                         h_electron_idx = j;
                         h_muon_idx = k;
                     }
                }
            }

            if (h_electron_idx != -1) {
                hist_map.at(name).Higgs_selectionMass->Fill(best_H_candidate.M());
                hist_map.at(name).Z_recoiledMass->Fill((beam_vector - best_H_candidate).M());
                
                std::vector<TLorentzVector> remaining_leptons;
                for(size_t j=0; j<electrons.size(); ++j) if(j != h_electron_idx) remaining_leptons.push_back(electrons[j]);
                for(size_t j=0; j<muons.size(); ++j) if(j != h_muon_idx) remaining_leptons.push_back(muons[j]);
                
                if(remaining_leptons.size() == 2) {
                    TLorentzVector z_reconstructed = remaining_leptons[0] + remaining_lepton[1];
                    hist_map.at(name).Z_reconstructedMass->Fill(z_reconstructed.M());
                }
            }
        } // End of event loop
        
        file->Close();
        delete file;
    } // End of file loop

    // --- Plotting ---
    // (Plotting code is unchanged and correct)
    
    gApplication->Terminate(0);
}