#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TApplication.h> // Needed for clean ROOT termination
#include <iostream>
#include <cstdlib>        // Needed for std::exit
#include <filesystem>

using namespace std;
// set beginning time
auto start_time = std::chrono::high_resolution_clock::now();
float Z_MASS = 91.1876; // GeV
float Z_WINDOW = 10.0; // GeV


// Function cut for Higgs Mass >= 150 GeV that push Z bosons off-shell at centre-of-mass energy 240 GeV
void Z_off_shell_cut(TString inputfile="HLFV_125GeV.root", TString outputfile="HLFV_125GeV_Zoff.root") {
    
    // --- FIX 1: Convert TString to C-string using .Data() ---
    if (!std::filesystem::exists(inputfile.Data())) {
        cerr << "Input file does not exist: " << inputfile << endl;
        // It is safer to use simple return in interactive ROOT, 
        // but exit(1) works for batch jobs.
        exit(1); 
    }

    // check input file content is zombie
    TFile *f = TFile::Open(inputfile);
    if (!f || f->IsZombie()) {
        cerr << "Input file is corrupted (zombie): " << inputfile << endl;
        exit(1);
    }
    
    cout << "Applying Z off-shell cut on file: " << inputfile << endl;
    
    // Get the tree
    TTree *t = (TTree*)f->Get("Delphes");
    
    // Safety check: ensure tree exists
    if (!t) {
        cerr << "Error: Tree 'Delphes' not found in file!" << endl;
        f->Close();
        exit(1);
    }
    
    // --- SPEED OPTIMIZATION START ---
    // 1. Disable ALL branches first
    t->SetBranchStatus("*", 0);
    
    // 2. Enable ONLY the branches you actually use
    t->SetBranchStatus("Event_size", 1);
    t->SetBranchStatus("Electron*", 1); // Enables all branches starting with Electron
    t->SetBranchStatus("Muon*", 1);     // Enables all branches starting with Muon
    // --- SPEED OPTIMIZATION END ---

    // Define variables
    unordered_map<string, float[4]> vecbranchMap = {};
    unordered_map<string, int[4]> intBranchMap = {};
    
    // Make sure these branches actually exist before setting address
    if (t->GetBranch("Event_size"))  t->SetBranchAddress("Event_size", &intBranchMap["Event_size"]);
    if (t->GetBranch("Electron_size")) t->SetBranchAddress("Electron_size", &intBranchMap["Electron_size"]);
    if (t->GetBranch("Electron.PT")) t->SetBranchAddress("Electron.PT", &vecbranchMap["Electron.PT"]);
    if (t->GetBranch("Electron.Eta")) t->SetBranchAddress("Electron.Eta", &vecbranchMap["Electron.Eta"]);
    if (t->GetBranch("Electron.Phi")) t->SetBranchAddress("Electron.Phi", &vecbranchMap["Electron.Phi"]);
    if (t->GetBranch("Electron.Charge")) t->SetBranchAddress("Electron.Charge", &intBranchMap["Electron.Charge"]);
    if (t->GetBranch("Muon_size")) t->SetBranchAddress("Muon_size", &intBranchMap["Muon_size"]);
    if (t->GetBranch("Muon.PT")) t->SetBranchAddress("Muon.PT", &vecbranchMap["Muon.PT"]);
    if (t->GetBranch("Muon.Eta")) t->SetBranchAddress("Muon.Eta", &vecbranchMap["Muon.Eta"]);
    if (t->GetBranch("Muon.Phi")) t->SetBranchAddress("Muon.Phi", &vecbranchMap["Muon.Phi"]);
    if (t->GetBranch("Muon.Charge")) t->SetBranchAddress("Muon.Charge", &intBranchMap["Muon.Charge"]);

    // Create output file and tree
    TFile *f_out = TFile::Open(outputfile, "RECREATE");
    // Make new empty tree
    TTree *t_out;

    // Loop over events and apply cut
    Long64_t nentries = t->GetEntries();
    cout << "Processing " << nentries << " entries..." << endl;

    for (Long64_t i = 0; i < nentries; i++) {
        t->GetEntry(i);
        // Check branchMap for the first event
        cout << "Event " << i << " - BranchMap contents:" << endl;

        // First Cut: All leptons PT > 10 GeV
        // Second Cut: 4 leptons in the event
        // Third Cut: Each lepton is odd numbered (1,3,5...) in the collection
        // Fourth Cut: No Charge violation in the event
        // Fifth cut: Z windows +- 10 GeV around Z mass (91.1876 GeV)
        bool CutStatus[6] = {false, false, false, false, false, false};
        // Muons
        for (int j = 0; j < intBranchMap["Muon_size"][0]; j++) {
            if (vecbranchMap["Muon.PT"][j] <= 10.0) {
                continue;
            }
        }
        // Electrons
        for (int j = 0; j < intBranchMap["Electron_size"][0]; j++) {
            if (vecbranchMap["Electron.PT"][j] <= 10.0) {
                continue;
            }
        }

        // other cuts ...
        if (intBranchMap["Muon_size"][0] + intBranchMap["Electron_size"][0] != 4) {
            CutStatus[0] = true;
            continue;
        }
        if (intBranchMap["Muon_size"][0] % 2 == 0 || intBranchMap["Electron_size"][0] % 2 == 0) {
            CutStatus[0] = true;
            CutStatus[1] = true;
            continue;
        }
        int chargeSum = 0;
        int chargeprod = 1;
        for (int j = 0; j < intBranchMap["Muon_size"][0]; j+=1) {
            chargeSum += intBranchMap["Muon.Charge"][j];
            chargeprod *= intBranchMap["Muon.Charge"][j];
        }
        for (int j = 0; j < intBranchMap["Electron_size"][0]; j+=1) {
            chargeSum += intBranchMap["Electron.Charge"][j];
            chargeprod *= intBranchMap["Electron.Charge"][j];
        }
        if (chargeSum != 0 || chargeprod != 1) {
            CutStatus[0] = true;
            CutStatus[1] = true;
            CutStatus[2] = true;
            continue;
        }
        vector<float> leptonPairsMass;
        TLorentzVector muonVector, electtronVector, combinedVector;
        vector<string> pairindexes;
        for (int j = 0; j < intBranchMap["Muon_size"][0]; j+=1) {
            for (int k = 0; k < intBranchMap["Electron_size"][0]; k+=1) {
                if (intBranchMap["Muon.Charge"][j] * intBranchMap["Electron.Charge"][k] > 0) {
                    continue; // Same charge, skip
                }
                // Use TLorentzVector to calculate invariant mass
                muonVector.SetPtEtaPhiM(vecbranchMap["Muon.PT"][j], vecbranchMap["Muon.Eta"][j], vecbranchMap["Muon.Phi"][j], 0.105);
                electtronVector.SetPtEtaPhiM(vecbranchMap["Electron.PT"][k], vecbranchMap["Electron.Eta"][k], vecbranchMap["Electron.Phi"][k], 0.105);
                combinedVector = muonVector + electtronVector;
                float mass = combinedVector.M();
                leptonPairsMass.push_back(mass);
                pairindexes.push_back(to_string(j) + "-" + to_string(k));
            }
        }
        // Check the Mass that Nearest to Z mass
        float minDiff = 1e6;
        int minIndex = -1;
        for (size_t idx = 0; idx < leptonPairsMass.size(); idx++) {
            float diff = fabs(leptonPairsMass[idx] - Z_MASS);
            if (diff < minDiff) {
                minDiff = diff;
                minIndex = idx;
            }
        }
        if ((minIndex == -1) || (fabs(leptonPairsMass[minIndex] - Z_MASS) > Z_WINDOW)) {
            CutStatus[0] = true;
            CutStatus[1] = true;
            CutStatus[2] = true;
            CutStatus[3] = true;
        }
        // input all infomation to t_out
        t_out->


        
    }
    // test loop overhead time without calculation
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    cout << "Loop overhead time: " << elapsed.count() << " seconds" << endl;

    // Write output tree to file
    f_out->cd();
    t_out->Write();
    
    // Clean up
    cout << "Done. Output written to: " << outputfile << endl;
    f_out->Close();
    f->Close();
}