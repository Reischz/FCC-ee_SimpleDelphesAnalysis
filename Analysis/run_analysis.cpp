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
#include "Delphes.C" // Assumes Delphes class is available

// --- Physics Constants ---
const double ELECTRON_MASS = 0.000511; // GeV/c^2
const double MUON_MASS = 0.1057;       // GeV/c^2
const double HIGGS_MASS = 125.0;       // GeV/c^2
const double Z_MASS = 91.1876;         // GeV/c^2
const double BEAM_ENERGY = 240.0;      // GeV (for recoil mass)

// --- Helper Struct for Histograms ---
// To keep the code clean, we group histograms by dataset.
struct Histograms {
    TH1D* Z_selectionMass;
    TH1D* Higgs_recoiledMass;
    TH1D* Higgs_reconstructedMass;
    TH1D* Higgs_selectionMass;
    TH1D* Z_recoiledMass;
    TH1D* Z_reconstructedMass;
    
    // For cut report
    long long n_total = 0;
    long long n_pass_cut1 = 0;
    long long n_pass_cut2 = 0;
    long long n_pass_cut3 = 0;

    // Constructor to initialize histograms
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
void run_analysis() {
    // --- Configuration ---
    std::map<std::string, std::string> file_paths = {
        {"HZ4Lep", "Bg1_hz.root"},
        {"ZWW4Lep", "Bg2_zww.root"},
        {"HZ4LepLFV", "Sn1_hzLFV.root"}
    };
    
    // Create histograms for each dataset
    std::map<std::string, Histograms> hist_map;
    for (auto const& [name, path] : file_paths) {
        hist_map.emplace(name, Histograms(name));
    }
    
    TLorentzVector beam_vector(0, 0, 0, BEAM_ENERGY);

    // Loop over each file/dataset
    for (auto const& [name, path] : file_paths) {
        TFile* file = TFile::Open(path.c_str());
        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << path << std::endl;
            continue;
        }
        
        std::cout << "Processing file: " << path << " for dataset: " << name << std::endl;

        TTree* tree = (TTree*)file->Get("Delphes");
        Delphes* d = new Delphes(tree);
        long long n_entries = tree->GetEntries();
        hist_map.at(name).n_total = n_entries;

        // --- Event Loop ---
        for (long long i = 0; i < n_entries; ++i) {
            d->GetEntry(i);

            // --- Cut 1: Exactly 4 leptons ---
            if (d->Electron_size + d->Muon_size != 4) continue;
            hist_map.at(name).n_pass_cut1++;
            
            // --- Cut 2: Odd number of electrons and muons (signal signature) ---
            if (d->Electron_size % 2 == 0 || d->Muon_size % 2 == 0) continue;
            hist_map.at(name).n_pass_cut2++;

            // --- Cut 3: Total charge is zero and no charge product violation ---
            int total_charge = 0;
            int charge_product = 1;
            for (int j = 0; j < d->Electron_size; ++j) {
                total_charge += d->Electron_Charge[j];
                charge_product *= d->Electron_Charge[j];
            }
            for (int j = 0; j < d->Muon_size; ++j) {
                total_charge += d->Muon_Charge[j];
                charge_product *= d->Muon_Charge[j];
            }
            if (total_charge != 0 || charge_product != 1) continue;
            hist_map.at(name).n_pass_cut3++;

            // --- Particle Reconstruction with TLorentzVector ---
            std::vector<TLorentzVector> electrons, muons, other_leptons, single_lepton_type;
            
            // This logic identifies the LFV candidate lepton type (e or mu)
            bool is_electron_single = (d->Electron_size == 1);
            
            for (int j = 0; j < d->Electron_size; ++j) {
                TLorentzVector p;
                p.SetPtEtaPhiM(d->Electron_PT[j], d->Electron_Eta[j], d->Electron_Phi[j], ELECTRON_MASS);
                electrons.push_back(p);
                if (is_electron_single) single_lepton_type.push_back(p);
                else other_leptons.push_back(p);
            }
            for (int j = 0; j < d->Muon_size; ++j) {
                TLorentzVector p;
                p.SetPtEtaPhiM(d->Muon_PT[j], d->Muon_Eta[j], d->Muon_Phi[j], MUON_MASS);
                muons.push_back(p);
                if (!is_electron_single) single_lepton_type.push_back(p);
                else other_leptons.push_back(p);
            }

            // =========================================================================
            // === Strategy 1: Find Z candidate first, then reconstruct Higgs         ===
            // =========================================================================
            double best_Z_mass_diff = 1e9;
            TLorentzVector best_Z_candidate;
            int z_idx1 = -1, z_idx2 = -1;

            // Find the best opposite-sign, same-flavor lepton pair
            for (size_t j = 0; j < other_leptons.size(); ++j) {
                for (size_t k = j + 1; k < other_leptons.size(); ++k) {
                    double mass = (other_leptons[j] + other_leptons[k]).M();
                    if (abs(mass - Z_MASS) < best_Z_mass_diff) {
                        best_Z_mass_diff = abs(mass - Z_MASS);
                        best_Z_candidate = other_leptons[j] + other_leptons[k];
                        z_idx1 = j;
                        z_idx2 = k;
                    }
                }
            }
            
            if(z_idx1 != -1) { // If a Z candidate was found
                hist_map.at(name).Z_selectionMass->Fill(best_Z_candidate.M());
                hist_map.at(name).Higgs_recoiledMass->Fill((beam_vector - best_Z_candidate).M());
                
                // The remaining lepton is the LFV one from the "other_leptons"
                int lfv_idx = 3 - z_idx1 - z_idx2; // The index of the third lepton
                TLorentzVector higgs_reconstructed = single_lepton_type[0] + other_leptons[lfv_idx];
                hist_map.at(name).Higgs_reconstructedMass->Fill(higgs_reconstructed.M());
            }

            // =========================================================================
            // === Strategy 2: Find Higgs (LFV) candidate first, then reconstruct Z  ===
            // =========================================================================
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

            if(h_electron_idx != -1) { // If a Higgs candidate was found
                hist_map.at(name).Higgs_selectionMass->Fill(best_H_candidate.M());
                hist_map.at(name).Z_recoiledMass->Fill((beam_vector - best_H_candidate).M());
                
                // Find remaining two leptons to reconstruct Z
                std::vector<TLorentzVector> remaining_leptons;
                for(size_t j=0; j<electrons.size(); ++j) if(j != h_electron_idx) remaining_leptons.push_back(electrons[j]);
                for(size_t j=0; j<muons.size(); ++j) if(j != h_muon_idx) remaining_leptons.push_back(muons[j]);
                
                if(remaining_leptons.size() == 2) {
                    TLorentzVector z_reconstructed = remaining_leptons[0] + remaining_leptons[1];
                    hist_map.at(name).Z_reconstructedMass->Fill(z_reconstructed.M());
                }
            }
        } // End of event loop
        
        file->Close();
        delete file;
        delete d;
    } // End of file loop

    // --- Plotting ---
    std::cout << "\nPlotting histograms..." << std::endl;
    
    // Define plot configurations
    std::vector<std::string> plot_names = {
        "Z_selectionMass", "Higgs_recoiledMass", "Higgs_reconstructedMass",
        "Higgs_selectionMass", "Z_recoiledMass", "Z_reconstructedMass"
    };
    
    std::vector<std::string> dataset_order = {"HZ4LepLFV", "ZWW4Lep", "HZ4Lep"};
    std::vector<Color_t> colors = {kOrange, kAzure - 3, kGreen + 1};

    for (const auto& plot_name : plot_names) {
        TCanvas* c1 = new TCanvas("c1", "c1", 800, 600);
        THStack* hs = new THStack("hs", "");
        TLegend* legend = new TLegend(0.65, 0.7, 0.88, 0.88);
        
        double max_y = 0;

        for (size_t i = 0; i < dataset_order.size(); ++i) {
            const auto& name = dataset_order[i];
            TH1D* h = nullptr;
            
            // A bit verbose, but safe way to get the correct histogram
            if (plot_name == "Z_selectionMass") h = hist_map.at(name).Z_selectionMass;
            else if (plot_name == "Higgs_recoiledMass") h = hist_map.at(name).Higgs_recoiledMass;
            else if (plot_name == "Higgs_reconstructedMass") h = hist_map.at(name).Higgs_reconstructedMass;
            else if (plot_name == "Higgs_selectionMass") h = hist_map.at(name).Higgs_selectionMass;
            else if (plot_name == "Z_recoiledMass") h = hist_map.at(name).Z_recoiledMass;
            else if (plot_name == "Z_reconstructedMass") h = hist_map.at(name).Z_reconstructedMass;

            if (h) {
                // Normalize to unit area (like density=True)
                if (h->Integral() > 0) {
                    h->Scale(1.0 / h->Integral());
                }
                
                h->SetLineColor(colors[i]);
                h->SetLineWidth(2);
                hs->Add(h, "HIST");
                legend->AddEntry(h, name.c_str(), "l");
                if (h->GetMaximum() > max_y) {
                    max_y = h->GetMaximum();
                }
            }
        }
        
        hs->Draw("NOSTACK");
        hs->SetTitle(TString::Format("%s;GeV;Normalized Entries", plot_name.c_st()));
        hs->SetMaximum(max_y * 1.2);
        legend->Draw();
        
        c1->SaveAs(TString::Format("CFigure/%s.png", plot_name.c_str()));
        delete legend;
        delete hs;
        delete c1;
    }

    // --- Cut Report ---
    std::cout << "\n--- Cut Report ---" << std::endl;
    for (const auto& name : dataset_order) {
        const auto& data = hist_map.at(name);
        if (data.n_total == 0) continue;
        printf("--- %s ---\n", name.c_str());
        printf("  Initial events: %lld\n", data.n_total);
        printf("  Pass Cut 1 (4 leptons): %lld (%.2f%%)\n", data.n_pass_cut1, (double)data.n_pass_cut1 / data.n_total * 100.0);
        printf("  Pass Cut 2 (Flavor):    %lld (%.2f%%)\n", data.n_pass_cut2, (double)data.n_pass_cut2 / data.n_total * 100.0);
        printf("  Pass Cut 3 (Charge):    %lld (%.2f%%)\n", data.n_pass_cut3, (double)data.n_pass_cut3 / data.n_total * 100.0);
    }
    
    gApplication->Terminate(0);
}