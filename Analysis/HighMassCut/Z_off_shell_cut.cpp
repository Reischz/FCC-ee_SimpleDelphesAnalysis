#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TString.h>
#include <TApplication.h> 
#include <iostream>
#include <cstdlib>        
#include <filesystem>
#include <vector>
#include <map>
#include <chrono>
#include "TLorentzVector.h" 
#include "TH1.h"
#include "TH2.h"
#include "lib/selectionlist.h" // Defines EventContext (using Arrays)

using namespace std;
auto start_time = std::chrono::high_resolution_clock::now();

// =========================================================================
// 1. LEAF READER ADAPTER
// Reads raw values directly, bypassing missing Class Dictionaries
// =========================================================================
struct LeafReader {
    // Pointers to the raw leaves in the TTree
    TLeaf *Event_size = nullptr;
    TLeaf *Electron_size = nullptr;
    TLeaf *Muon_size = nullptr;
    TLeaf *Particle_size = nullptr;
    TLeaf *MET_Phi = nullptr;

    TLeaf *Ele_PT = nullptr;
    TLeaf *Ele_Eta = nullptr;
    TLeaf *Ele_Phi = nullptr;
    TLeaf *Ele_Chg = nullptr;

    TLeaf *Mu_PT = nullptr;
    TLeaf *Mu_Eta = nullptr;
    TLeaf *Mu_Phi = nullptr;
    TLeaf *Mu_Chg = nullptr;

    TLeaf *Par_PT = nullptr;
    TLeaf *Par_Eta = nullptr;
    TLeaf *Par_Phi = nullptr;
    TLeaf *Par_PID = nullptr;
    TLeaf *Par_Status = nullptr;
    TLeaf *Par_M1 = nullptr;
    TLeaf *Par_M2 = nullptr;
    TLeaf *Par_D1 = nullptr;
    TLeaf *Par_D2 = nullptr;

    // Connects leaves to the tree
    void Init(TTree* t) {
        // Helper to get leaf safely
        auto get = [&](const char* name) { 
            TLeaf* l = t->GetLeaf(name);
            if (!l) cout << "Warning: Leaf '" << name << "' not found." << endl;
            return l;
        };

        Event_size = get("Event_size");
        Electron_size = get("Electron_size");
        Muon_size = get("Muon_size");
        Particle_size = get("Particle_size");
        MET_Phi = get("MissingET.Phi");

        Ele_PT = get("Electron.PT");
        Ele_Eta = get("Electron.Eta");
        Ele_Phi = get("Electron.Phi");
        Ele_Chg = get("Electron.Charge");

        Mu_PT = get("Muon.PT");
        Mu_Eta = get("Muon.Eta");
        Mu_Phi = get("Muon.Phi");
        Mu_Chg = get("Muon.Charge");

        Par_PT = get("Particle.PT");
        Par_Eta = get("Particle.Eta");
        Par_Phi = get("Particle.Phi");
        Par_PID = get("Particle.PID");
        Par_Status = get("Particle.Status");
        Par_M1 = get("Particle.M1");
        Par_M2 = get("Particle.M2");
        Par_D1 = get("Particle.D1");
        Par_D2 = get("Particle.D2");

    }

    // Reads current entry from Leaves into the EventContext
    void ReadEntry(EventContext& ev) {
        // Scalars
        if(Event_size) ev.Event_size = (int)Event_size->GetValue();
        if(MET_Phi)    ev.MET_Phi = (float)MET_Phi->GetValue();
        
        // Electrons
        if(Electron_size) {
            ev.Electron_size = (int)Electron_size->GetValue();
            // Safety clamp to 20 (assuming EventContext array size is 20)
            int count = std::min(ev.Electron_size, 20); 
            for(int i=0; i<count; ++i) {
                if(Ele_PT)  ev.Electron_PT[i]  = (float)Ele_PT->GetValue(i);
                if(Ele_Eta) ev.Electron_Eta[i] = (float)Ele_Eta->GetValue(i);
                if(Ele_Phi) ev.Electron_Phi[i] = (float)Ele_Phi->GetValue(i);
                if(Ele_Chg) ev.Electron_Charge[i] = (int)Ele_Chg->GetValue(i);
            }
        }

        // Muons
        if(Muon_size) {
            ev.Muon_size = (int)Muon_size->GetValue();
            int count = std::min(ev.Muon_size, 20);
            for(int i=0; i<count; ++i) {
                if(Mu_PT)  ev.Muon_PT[i]  = (float)Mu_PT->GetValue(i);
                if(Mu_Eta) ev.Muon_Eta[i] = (float)Mu_Eta->GetValue(i);
                if(Mu_Phi) ev.Muon_Phi[i] = (float)Mu_Phi->GetValue(i);
                if(Mu_Chg) ev.Muon_Charge[i] = (int)Mu_Chg->GetValue(i);
            }
        }

        // Particles
        if(Particle_size) {
            ev.Particle_size = (int)Particle_size->GetValue();
            int count = std::min(ev.Particle_size, 50); // Assumes Particle array size is larger
            for(int i=0; i<count; ++i) {
                if(Par_PT)  ev.Particle_PT[i]  = (float)Par_PT->GetValue(i);
                if(Par_Eta) ev.Particle_Eta[i] = (float)Par_Eta->GetValue(i);
                if(Par_Phi) ev.Particle_Phi[i] = (float)Par_Phi->GetValue(i);
                if(Par_PID) ev.Particle_PID[i] = (int)Par_PID->GetValue(i);
                if(Par_Status) ev.Particle_Status[i] = (int)Par_Status->GetValue(i);
                if(Par_M1) ev.Particle_M1[i] = (int)Par_M1->GetValue(i);
                if(Par_M2) ev.Particle_M2[i] = (int)Par_M2->GetValue(i);
                if(Par_D1) ev.Particle_D1[i] = (int)Par_D1->GetValue(i);
                if(Par_D2) ev.Particle_D2[i] = (int)Par_D2->GetValue(i);
            }
        }
    }
};

// =========================================================================
// 2. HELPER: Histogram Manager
// =========================================================================
struct HistManager {
    std::map<TString, TH1*> h1;
    std::map<TString, TH2*> h2;
    TDirectory* dir = nullptr;

    void SetDirectory(TDirectory* d) { dir = d; }

    TH1F* Book1D(TString name, TString title, int nbins, double min, double max) {
        if (dir) dir->cd();
        TH1F* h = new TH1F(name, title, nbins, min, max);
        h->SetDirectory(dir);
        h1[name] = h;
        return h;
    }

    TH1F* Book1DLogX(TString name, TString title, int nbins, double min, double max) {
        if (dir) dir->cd();
        TH1F* h = new TH1F(name, title, nbins, min, max);
        h->SetXaxis(new TLogAxis());
        h->SetDirectory(dir);
        h1[name] = h;
        return h;
    }

    TH2F* Book2D(TString name, TString title, int nxbins, double xmin, double xmax, int nybins, double ymin, double ymax) {
        if (dir) dir->cd();
        TH2F* h = new TH2F(name, title, nxbins, xmin, xmax, nybins, ymin, ymax);
        h->SetDirectory(dir);
        h2[name] = h;
        return h;
    }

    void Fill1D(TString name, double val) {
        if (h1.count(name)) h1[name]->Fill(val);
    }

    void Fill2D(TString name, double valx, double valy) {
        if (h2.count(name)) h2[name]->Fill(valx, valy);
    }
};

// =========================================================================
// 3. PIPELINE CONFIGURATION
// =========================================================================
using AnalysisStep = std::pair<AnalysisModule*, bool>;

std::vector<AnalysisStep> ConfigurePipeline() {
    return {
        { new NonSelection()          , true },
        { new Lepton_PT()             , true },
        { new FinalState_4Leptons()   , true },
        { new Lepton_Odd()            , true },
        { new Charge_Violation()      , true },
        { new PairSelection_offshell(), true },
        { new NotZ_MassThreshold()    , true }
    };
}

// =========================================================================
// 4. MAIN MACRO
// =========================================================================
void Z_off_shell_cut(
    TString inputfile = "Prelim_sample/HLFV_160GeV.root", 
    TString outputfile = "Prelim_result/HLFV_160GeV_Zoff.root", 
    TString TreeOutput = "Prelim_result/HLFV_160GeV_AdditionalTree.root",
    bool testMode = true
) { 
    auto start_time = std::chrono::high_resolution_clock::now();

    // --- Input Validation ---
    if (!std::filesystem::exists(inputfile.Data())) {
        cerr << "Input file missing: " << inputfile << endl; exit(1);
    }

    TFile *fIn = TFile::Open(inputfile);
    if (!fIn || fIn->IsZombie()) { cerr << "File corrupted." << endl; exit(1); }
    
    TTree *tIn = (TTree*)fIn->Get("Delphes");
    if (!tIn) { cerr << "Tree 'Delphes' not found." << endl; exit(1); }
    
    // --- Setup Leaf Reader (No SetBranchAddress!) ---
    // Note: We do NOT define SetBranchAddress here. 
    // We rely on TLeaf which works even without dictionaries.
    tIn->SetBranchStatus("*", 1); // Enable all for Leaf access
    LeafReader reader;
    reader.Init(tIn);

    // --- Output Setup ---
    TFile *fTreeOut = TFile::Open(TreeOutput, "RECREATE");
    TTree *tOut = new TTree("Selection Results", "Z off-shell results");
    
    EventContext ev; // This comes from lib/selectionlist.h
    defaultParameters params;
    auto pipeline = ConfigurePipeline();
    AnalysisModule* LastVerifyGen = new Verify_Generator();

    int dummy = 0;
    for (auto& step : pipeline) {
        if (step.second) {
            TString bName = TString::Format("Status_%02d_%s", dummy, step.first->getName().c_str());
            tOut->Branch(bName, &ev.CutStatus[dummy], bName + "/I");
            dummy++;
        }
    }
    tOut->Branch("NearestZ_Mass", &ev.NearestZ_Mass, "NearestZ_Mass/F");
    tOut->Branch("OtherPair_Mass", &ev.OtherPair_Mass, "OtherPair_Mass/F");

    // --- Histogram Setup ---
    TFile *fHistOut = TFile::Open(outputfile, "RECREATE");
    TDirectory *histDir = fHistOut->mkdir("histgramtree");
    HistManager hm;
    hm.SetDirectory(histDir);

    dummy = 0;
    for (auto& step : pipeline) {
        if (!step.second) continue;
        TString stepName = step.first->getName();
        TString prefix = TString::Format("%02d_", dummy);
        
        hm.Book1D(prefix + "Lepton.PT-" + stepName, "PT;GeV", 100, 0, 200);
        hm.Book1D(prefix + "Lepton.Eta-" + stepName, "Eta", 100, -3, 3);
        hm.Book1D(prefix + "Lepton.Phi-" + stepName, "Phi", 100, -5, 5);

        if (step.first->isPairedLepton) {
            hm.Book1D(prefix + "NearestZ_Mass-" + stepName, "M_Z1;GeV", 100, 0, 200);
            hm.Book1D(prefix + "OtherPair_Mass-" + stepName, "M_Z2;GeV", 100, 0, 200);
            hm.Book1D(prefix + "NotZ_dR-" + stepName, "dR", 50, 0, 6);
            hm.Book1D(prefix + "NotZ_dPhi-" + stepName, "dPhi", 50, 0, 3.5);
            hm.Book2D(prefix + "MassPairHeatmap-" + stepName, "Mass Map", 200, 0, 200, 200, 0, 200);
        }
        dummy++;
    }

    hm.Book2D("SFSC_dR_Heatmap", "Gen vs Reco dR", 100, 0, 10, 50, 0, 10);
    hm.Book1D("SFSC_dR_Ratio", "Reco/Gen dR Ratio", 300, 0, 3);
    hm.Book2D("Matching_ThreeLep_Opposite_Heatmap", "Matching Check", 2, 0, 2, 2, 0, 2);
    hm.Book1D("Matching_SingleLep_dRtoGen", "Single Lep dR to Gen", 200, 0, 2);
    hm.Book1D("Matching_ThreeLep_dRtoGen", "Three Lep dR to Gen", 200, 0, 2);
    hm.Book1D("FreeAllLep_dRtoGen", "All Lep dR to Gen", 200, 0, 2);
    // =========================================================================

    // --- Event Loop ---
    Long64_t nentries = tIn->GetEntries();
    cout << "Processing " << nentries << " entries..." << endl;
    
    vector<int> selection_counts(10, 0);
    int stats_single=0, stats_three=0, stats_perf=0, stats_opp=0, stats_FA=0;

    if (testMode) nentries = 10000; // For quick tests
    for (Long64_t i = 0; i < nentries; i++) {
        if (i % 100000 == 0) cout << " Processed " << i << " events..." << endl;
        
        // 1. Get Entry (Loads Leaves)
        tIn->GetEntry(i);
        ev.reset();

        // 2. Read Leaves into EventContext (Array)
        reader.ReadEntry(ev);

        // 3. Run Pipeline
        dummy = 0;
        for (auto& step : pipeline) {
            if (!step.second) continue;

            step.first->process(ev, params);

            ev.CutStatus[ev.CurrentCut] = ev.PassThisCut ? 1 : 0;
            if (ev.PassThisCut) selection_counts[dummy]++;
            if (!ev.PassThisCut) break; 

            TString stepName = step.first->getName();
            TString prefix = TString::Format("%02d_", dummy);

            // Fill Histograms using the EventContext (which now has valid data)
            for(int k=0; k<ev.Electron_size; k++) hm.Fill1D(prefix + "Lepton.PT-" + stepName, ev.Electron_PT[k]);
            for(int k=0; k<ev.Muon_size; k++) hm.Fill1D(prefix + "Lepton.PT-" + stepName, ev.Muon_PT[k]);
            
            for(int k=0; k<ev.Electron_size; k++) hm.Fill1D(prefix + "Lepton.Eta-" + stepName, ev.Electron_Eta[k]);
            for(int k=0; k<ev.Muon_size; k++) hm.Fill1D(prefix + "Lepton.Eta-" + stepName, ev.Muon_Eta[k]);

            for(int k=0; k<ev.Electron_size; k++) hm.Fill1D(prefix + "Lepton.Phi-" + stepName, ev.Electron_Phi[k]);
            for(int k=0; k<ev.Muon_size; k++) hm.Fill1D(prefix + "Lepton.Phi-" + stepName, ev.Muon_Phi[k]);

            if (step.first->isPairedLepton) {
                hm.Fill1D(prefix + "NearestZ_Mass-" + stepName, ev.NearestZ_Mass);
                hm.Fill1D(prefix + "OtherPair_Mass-" + stepName, ev.OtherPair_Mass);
                hm.Fill1D(prefix + "NotZ_dR-" + stepName, ev.NotZ_dR);
                hm.Fill1D(prefix + "NotZ_dPhi-" + stepName, ev.NotZ_dPhi);
                hm.Fill2D(prefix + "MassPairHeatmap-" + stepName, ev.NearestZ_Mass, ev.OtherPair_Mass);
            }
            
            ev.CurrentCut++;
            dummy++;

            if (stepName == "NotZ_MassThreshold") {
                LastVerifyGen->process(ev, params);
                hm.Fill2D("SFSC_dR_Heatmap", ev.SFSC_GendR, ev.SFSC_RecodR);
                if (ev.SFSC_GendR > 0) hm.Fill1D("SFSC_dR_Ratio", ev.SFSC_RecodR / ev.SFSC_GendR);
                hm.Fill2D("Matching_ThreeLep_Opposite_Heatmap", ev.Matching_ThreeLepSide, ev.Matching_OppositeLep);
                hm.Fill1D("Matching_SingleLep_dRtoGen", ev.SingleLep_dRtoGen);
                hm.Fill1D("Matching_ThreeLep_dRtoGen", ev.ThreeLep_dRtoGen);
                
                if (ev.Matching_SingleLepSide) stats_single++;
                if (ev.Matching_ThreeLepSide) stats_three++;
                if (ev.Matching_Perfect) stats_perf++;
                if (ev.Matching_OppositeLep) stats_opp++;
                for (int k=0; k<ev.FreeAllLep_dRtoGen.size(); k++) {
                    hm.Fill1D("FreeAllLep_dRtoGen", ev.FreeAllLep_dRtoGen[k]);
                    if (ev.FreeAllLep_MatchStatus[k]) stats_FA++;
                }
            }
        }
        tOut->Fill();
    }

    // --- Cleanup & Summary ---
    cout << "\nSelection Summary:" << endl;
    for(int k=0; k<pipeline.size(); k++) cout << " Stage " << k << ": " << selection_counts[k] 
                                            << ", " << pipeline[k].first->getName() << endl;
    for (auto& step : pipeline) delete step.first;
    delete LastVerifyGen;


    fTreeOut->cd();
    tOut->Write();
    fTreeOut->Close();

    fHistOut->cd();
    TH1F *hStats = new TH1F("Matching_Bars_chart", "Matching Stats", 4, 0, 4);
    hStats->SetBinContent(1, stats_single); hStats->GetXaxis()->SetBinLabel(1, "Single");
    hStats->SetBinContent(2, stats_three);  hStats->GetXaxis()->SetBinLabel(2, "Three");
    hStats->SetBinContent(3, stats_perf);   hStats->GetXaxis()->SetBinLabel(3, "Perfect");
    hStats->SetBinContent(4, stats_FA);     hStats->GetXaxis()->SetBinLabel(4, "FreeAll");
    hStats->SetDirectory(histDir);
    
    histDir->Write();
    fHistOut->Close();
    fIn->Close();

    cout << "Done." << endl;
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    cout << "Total execution time: " << elapsed.count() << " seconds" << endl;
    gApplication->Terminate();
}