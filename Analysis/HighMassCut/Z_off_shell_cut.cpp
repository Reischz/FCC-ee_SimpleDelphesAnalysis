#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TApplication.h> // Needed for clean ROOT termination
#include <iostream>
#include <cstdlib>        // Needed for std::exit
#include <filesystem>
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h" // This is the one currently missing
#include "TH1.h"
#include "TH2.h"
// Add any other ROOT headers you use (TH1F.h, TChain.h, etc.)

using namespace std;
// set beginning time
auto start_time = std::chrono::high_resolution_clock::now();

// First Cut: All leptons PT > 10 GeV
// Second Cut: 4 leptons in the event
// Third Cut: Each lepton is odd numbered (1,3,5...) in the collection
// Fourth Cut: No Charge violation in the event
// Fifth cut: Z windows +- 10 GeV around Z mass (91.1876 GeV)
// ==========================================
//  Data Container Definition
// ==========================================
struct defaultParameters {
    static constexpr float Electron_MASS = 0.000511; // GeV
    static constexpr float Muon_MASS = 0.105658; // GeV
    static constexpr float Z_MASS = 91.1876; // GeV
    static constexpr float Z_WINDOW = 10.0; // GeV
    static constexpr float Min_ElecPT = 10.0; // GeV
    static constexpr float Min_MuonPT = 10.0; // GeV
    static constexpr float dRCut = 2.0; // dR cut for Not Z lepton pair
    static constexpr float NotZ_MassCut = 75.0; // dPhi cut for Not Z lepton pair and MET
};
struct EventContext {
    // --- Inputs (e.g., from TTree) ---
    int Event_size;
    int Electron_size;
    float Electron_PT[4]; // assuming max 4 electrons
    float Electron_Eta[4];
    float Electron_Phi[4];
    int Electron_Charge[4];

    int Muon_size;
    float Muon_PT[4];
    float Muon_Eta[4];
    float Muon_Phi[4];
    int Muon_Charge[4];

    float MET_PT;
    float MET_Phi;

    int CutStatus[10]; // To store cut results
    int CurrentCut;
    bool PassThisCut;

    float NearestZ_Mass;
    float OtherPair_Mass;
    int Z_PairIndexSum;
    float NotZ_dR;
    float NotZ_dPhi;
    float NotZ_EleMET_dPhi;
    float NotZ_MuMET_dPhi;

    // Constructor to reset values per loop
    void reset() {
        Event_size = 0;
        Electron_size = 0;
        Muon_size = 0;
        CurrentCut = 0;
        PassThisCut = true;
        NearestZ_Mass = -1.0;
        OtherPair_Mass = -1.0;
        Z_PairIndexSum = -1;
        NotZ_EleMET_dPhi = -1.0;
        NotZ_MuMET_dPhi = -1.0;
        NotZ_dR = -1.0;
        NotZ_dPhi = -1.0;
        for (int i = 0; i < 10; ++i) {
            CutStatus[i] = 0;
        }
    }
    // Function to print current state (for debugging)
    void print() {
        cout << " Event_size: " << Event_size << endl;
        cout << " Electron_size: " << Electron_size << endl;
        cout << " Muon_size: " << Muon_size << endl;
        cout << " NearestZ_Mass: " << NearestZ_Mass << endl;
        cout << " OtherPair_Mass: " << OtherPair_Mass << endl;
        cout << " Z_PairIndexSum: " << Z_PairIndexSum << endl;
        cout << " CutStatus: ";
        for (int i = 0; i < 10; ++i) {
            cout << CutStatus[i] << " ";
        }
        // PT, Eta, Phi, Charge of Electrons
        cout << "\n Electrons: ";
        for (int i = 0; i < Electron_size; ++i) {
            cout << "(PT: " << Electron_PT[i] << ", Eta: " << Electron_Eta[i] << ", Phi: " << Electron_Phi[i] << ", Charge: " << Electron_Charge[i] << ") ";
        }
        cout << "\n Muons: ";
        for (int i = 0; i < Muon_size; ++i) {
            cout << "(PT: " << Muon_PT[i] << ", Eta: " << Muon_Eta[i] << ", Phi: " << Muon_Phi[i] << ", Charge: " << Muon_Charge[i] << ") ";
        }
        cout << endl;
    }
};
// ==========================================
//  Mother of all analysis modules
// ==========================================
class AnalysisModule {
    protected:
        std::string name;
    public:
        AnalysisModule(std::string n) : name(n) {}
        virtual ~AnalysisModule() {}
        virtual void process(EventContext &data, const defaultParameters &params) = 0; // Pure virtual
        virtual std::string getName() const { return name; }
};

// ==========================================
// Analysis Module
// ==========================================
class Lepton_PT : public AnalysisModule {
    public:
        Lepton_PT() : AnalysisModule("Lepton_PT") {}

        void process(EventContext &data, const defaultParameters &params) override {
            for (int i = 0; i < data.Electron_size; ++i) {
                if (data.Electron_PT[i] < params.Min_ElecPT) {
                    data.PassThisCut = false;
                    return;
                }
            }
            for (int i = 0; i < data.Muon_size; ++i) {
                if (data.Muon_PT[i] < params.Min_MuonPT) {
                    data.PassThisCut = false;
                    return;
                }
            }
            return;
        }
}; 
class FinalState_4Leptons : public AnalysisModule {
    public:
        FinalState_4Leptons() : AnalysisModule("FinalState_4Leptons") {}

        void process(EventContext &data, const defaultParameters &params) override {
            if ((data.Electron_size + data.Muon_size) != 4) {
                data.PassThisCut = false;
                return;
            }
            return;
        }
};
class Lepton_Odd : public AnalysisModule {
    public:
        Lepton_Odd() : AnalysisModule("Lepton_Odd") {}

        void process(EventContext &data, const defaultParameters &params) override {
            if (data.Electron_size % 2 == 0 || data.Muon_size % 2 == 0) {
                data.PassThisCut = false;
            }
            return;
        }
};
class Charge_Violation : public AnalysisModule {
    public:
        Charge_Violation() : AnalysisModule("Charge_Violation") {}

        void process(EventContext &data, const defaultParameters &params) override {
            int chargeSum = 0;
            int chargeprod = 1;
            for (int j = 0; j < data.Muon_size; j+=1) {
                chargeSum += data.Muon_Charge[j];
                chargeprod *= data.Muon_Charge[j];
            }
            for (int j = 0; j < data.Electron_size; j+=1) {
                chargeSum += data.Electron_Charge[j];
                chargeprod *= data.Electron_Charge[j];
            }
            if (chargeSum != 0 || chargeprod != 1) {
                data.PassThisCut = false;
            }
            return;
        }
};
class Z_Window : public AnalysisModule {
    public:
        Z_Window() : AnalysisModule("Z_Window") {}

        void process(EventContext &data, const defaultParameters &params) override {
            vector<float> leptonPairsMass;
            TLorentzVector lepton1Vector, lepton2Vector, combinedVector;
            vector<int> pairindexes;
            // 3 Muon + 1 Electron
            if (data.Muon_size == 3 && data.Electron_size == 1) {
                // Doing Muon pairs Selection
                for (int i =0; i<3;i++){
                    for (int j = i+1; j<3;j++){
                        if (data.Muon_Charge[i] * data.Muon_Charge[j] > 0) {
                            continue; // Same charge, skip
                        }
                        // Use TLorentzVector to calculate invariant mass
                        lepton1Vector.SetPtEtaPhiM(data.Muon_PT[i], data.Muon_Eta[i], data.Muon_Phi[i], params.Muon_MASS);
                        lepton2Vector.SetPtEtaPhiM(data.Muon_PT[j], data.Muon_Eta[j], data.Muon_Phi[j], params.Muon_MASS);
                        combinedVector = lepton1Vector + lepton2Vector;
                        float mass = combinedVector.M();
                        leptonPairsMass.push_back(mass);
                        pairindexes.push_back(i + j);
                    }
                }
            }
            // 3 Electron + 1 Muon
            else if (data.Electron_size == 3 && data.Muon_size == 1) {
                // Doing Electron pairs Selection
                for (int i =0; i<3;i++){
                    for (int j = i+1; j<3;j++){
                        if (data.Electron_Charge[i] * data.Electron_Charge[j] > 0) {
                            continue; // Same charge, skip
                        }
                        // Use TLorentzVector to calculate invariant mass
                        lepton1Vector.SetPtEtaPhiM(data.Electron_PT[i], data.Electron_Eta[i], data.Electron_Phi[i], params.Electron_MASS);
                        lepton2Vector.SetPtEtaPhiM(data.Electron_PT[j], data.Electron_Eta[j], data.Electron_Phi[j], params.Electron_MASS);
                        combinedVector = lepton1Vector + lepton2Vector;
                        float mass = combinedVector.M();
                        leptonPairsMass.push_back(mass);
                        pairindexes.push_back(i + j);
                    }
                }
            }
            // Check the Mass that Nearest to Z mass
            float minDiff = 1e6;
            int minIndex = -1;
            for (size_t idx = 0; idx < leptonPairsMass.size(); idx++) {
                float diff = fabs(leptonPairsMass[idx] - params.Z_MASS);
                if (diff < minDiff) {
                    minDiff = diff;
                    minIndex = idx;
                }
            }
            data.Z_PairIndexSum = pairindexes[minIndex];
            if ((minIndex == -1) || (fabs(leptonPairsMass[minIndex] - params.Z_MASS) > params.Z_WINDOW)) {
                data.PassThisCut = false;
            } else {
                data.NearestZ_Mass = leptonPairsMass[minIndex];
                int leftindex=3-data.Z_PairIndexSum;
                // Find the other pair mass
                if (data.Electron_size>data.Muon_size){
                    lepton1Vector.SetPtEtaPhiM(data.Muon_PT[0], data.Muon_Eta[0], data.Muon_Phi[0], params.Muon_MASS);
                    lepton2Vector.SetPtEtaPhiM(data.Electron_PT[leftindex], data.Electron_Eta[leftindex], data.Electron_Phi[leftindex], params.Electron_MASS);
                    combinedVector = lepton1Vector + lepton2Vector;
                    data.OtherPair_Mass = combinedVector.M();
                } else {
                    lepton1Vector.SetPtEtaPhiM(data.Electron_PT[0], data.Electron_Eta[0], data.Electron_Phi[0], params.Electron_MASS);
                    lepton2Vector.SetPtEtaPhiM(data.Muon_PT[leftindex], data.Muon_Eta[leftindex], data.Muon_Phi[leftindex], params.Muon_MASS);
                    combinedVector = lepton1Vector + lepton2Vector;
                    data.OtherPair_Mass = combinedVector.M();
                }
            }
            return;
        }
};
class NotZ_dR : public AnalysisModule {
    public:
        NotZ_dR() : AnalysisModule("NotZ_dR") {}

        void process(EventContext &data, const defaultParameters &params) override {
            if (!data.CutStatus[data.CurrentCut-1]){
                return;
            }
            // Identify the not Z pair leptons
            TLorentzVector MuVector, EleVector;
            int EleIndex=0, MuIndex=0;
            if (data.Electron_size > data.Muon_size) {
                // 3 Electrons + 1 Muon case
                EleIndex = 3-data.Z_PairIndexSum;
            } else {
                // 3 Muons + 1 Electron case
                MuIndex = 3-data.Z_PairIndexSum;
            }
            MuVector.SetPtEtaPhiM(data.Muon_PT[MuIndex], data.Muon_Eta[MuIndex], data.Muon_Phi[MuIndex], params.Muon_MASS);
            EleVector.SetPtEtaPhiM(data.Electron_PT[EleIndex], data.Electron_Eta[EleIndex], data.Electron_Phi[EleIndex], params.Electron_MASS);
            float dR = MuVector.DeltaR(EleVector);
            float dPhi = fabs(MuVector.DeltaPhi(EleVector));
            data.NotZ_dR = dR;
            data.NotZ_dPhi = dPhi;
            if (dR < params.dRCut) {
                data.PassThisCut = false;
            }
            return;
        }
};
class NotZ_MET_dPhi : public AnalysisModule {
    public:
        NotZ_MET_dPhi() : AnalysisModule("NotZ_MET_dPhi") {}

        void process(EventContext &data, const defaultParameters &params) override {
            // Placeholder for future implementation
            if (!data.CutStatus[data.CurrentCut-1]){
                return;
            }
            // Identify the not Z pair leptons
            int EleIndex=0, MuIndex=0;
            if (data.Electron_size > data.Muon_size) {
                // 3 Electrons + 1 Muon case
                EleIndex = 3-data.Z_PairIndexSum;
            } else {
                // 3 Muons + 1 Electron case
                MuIndex = 3-data.Z_PairIndexSum;
            }
            float METPhi=data.MET_Phi;
            // Calculate dPhi between Not Z lepton pair and MET
            // Logic: Calculate raw difference -> Normalize to [-pi, pi] -> Take absolute value
            data.NotZ_EleMET_dPhi = fabs(TVector2::Phi_mpi_pi(data.Electron_Phi[EleIndex] - METPhi));
            data.NotZ_MuMET_dPhi  = fabs(TVector2::Phi_mpi_pi(data.Muon_Phi[MuIndex] - METPhi));
            return;
        }
};
class NotZ_MassThreshold : public AnalysisModule {
    public:
        NotZ_MassThreshold() : AnalysisModule("NotZ_MassThreshold") {}

        void process(EventContext &data, const defaultParameters &params) override {
            if (!data.CutStatus[data.CurrentCut-1]){
                return;
            }
            if (data.OtherPair_Mass < params.NotZ_MassCut) {
                data.PassThisCut = false;
            }
            return;
        }
};
// ==========================================
// Workflow Modular Design
// ==========================================
using AnalysisStep = std::pair<AnalysisModule*, bool>;
std::vector<AnalysisStep> ConfigurePipeline() {
    // Return the list directly!
    return {
        { new Lepton_PT()           ,    true  },
        { new FinalState_4Leptons() ,    true  }, 
        { new Lepton_Odd()          ,    true  },
        { new Charge_Violation()    ,    true  },
        { new Z_Window()            ,    true  },
        { new NotZ_dR()             ,    true  }, // Placeholder for future modules
        { new NotZ_MET_dPhi()       ,    true  }, // Disabled module example
        { new NotZ_MassThreshold()  ,    true  }
    };
}
// ==========================================
// Main Execution (ROOT Macro)
// ==========================================
void Z_off_shell_cut(TString inputfile="HLFV_125GeV.root", TString outputfile="HLFV_125GeV_Zoff.root", TString TreeOutput="HLFV_125GeV_AdditionalTree.root") {
    
    // ===============================
    // Setting up input (Root) file
    // ===============================
    // check input file existence
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
    // t->SetBranchStatus("MissingET.MET", 1);
    t->SetBranchStatus("MissingET.Phi", 1);
    // --- SPEED OPTIMIZATION END ---

    // Define variables
    EventContext currentEvent;
    defaultParameters params;
    auto pipeline=ConfigurePipeline();
    
    // Make sure these branches actually exist before setting address
    if (t->GetBranch("Event_size"))  t->SetBranchAddress("Event_size", &currentEvent.Event_size);
    if (t->GetBranch("Electron_size")) t->SetBranchAddress("Electron_size", &currentEvent.Electron_size);
    if (t->GetBranch("Electron.PT")) t->SetBranchAddress("Electron.PT", &currentEvent.Electron_PT);
    if (t->GetBranch("Electron.Eta")) t->SetBranchAddress("Electron.Eta", &currentEvent.Electron_Eta);
    if (t->GetBranch("Electron.Phi")) t->SetBranchAddress("Electron.Phi", &currentEvent.Electron_Phi);
    if (t->GetBranch("Electron.Charge")) t->SetBranchAddress("Electron.Charge", &currentEvent.Electron_Charge);
    if (t->GetBranch("Muon_size")) t->SetBranchAddress("Muon_size", &currentEvent.Muon_size);
    if (t->GetBranch("Muon.PT")) t->SetBranchAddress("Muon.PT", &currentEvent.Muon_PT);
    if (t->GetBranch("Muon.Eta")) t->SetBranchAddress("Muon.Eta", &currentEvent.Muon_Eta);
    if (t->GetBranch("Muon.Phi")) t->SetBranchAddress("Muon.Phi", &currentEvent.Muon_Phi);
    if (t->GetBranch("Muon.Charge")) t->SetBranchAddress("Muon.Charge", &currentEvent.Muon_Charge);
    if (t->GetBranch("MissingET.Phi")) t->SetBranchAddress("MissingET.Phi", &currentEvent.MET_Phi);

    //================================
    // Setting up new tree for contains cut results
    //================================
    TFile *tfile_out = TFile::Open(TreeOutput, "RECREATE");
    tfile_out->cd();
    TTree *t_out = new TTree("Selection Results", "Tree with Z off-shell cut results");
    int dummy = 0;
     // Create branches for each cut in the pipeline
    for (auto& step : pipeline) {
        if (step.second) { // If module is active
            TString branchName = TString::Format("Status_%02d_%s", dummy, step.first->getName().c_str()); 
            // e.g., Cut_00_Lepton_PT, Cut_1_FinalState_4Leptons, etc.
            t_out->Branch(branchName, &currentEvent.CutStatus[dummy], (branchName + "/I"));
            dummy++;
        }
    }
    t_out->Branch("NearestZ_Mass", &currentEvent.NearestZ_Mass, "NearestZ_Mass/F");
    t_out->Branch("OtherPair_Mass", &currentEvent.OtherPair_Mass, "OtherPair_Mass/F");
    t_out->Branch("Z_PairIndexSum", &currentEvent.Z_PairIndexSum, "Z_PairIndexSum/I");
    t_out->Branch("NotZ_dR", &currentEvent.NotZ_dR, "NotZ_dR/F");
    t_out->Branch("NotZ_dPhi", &currentEvent.NotZ_dPhi, "NotZ_dPhi/F");
    t_out->Branch("NotZ_EleMET_dPhi", &currentEvent.NotZ_EleMET_dPhi, "NotZ_EleMET_dPhi/F");
    t_out->Branch("NotZ_MuMET_dPhi", &currentEvent.NotZ_MuMET_dPhi, "NotZ_MuMET_dPhi/F");
    // ==============================
    
    //=============================
    // Histogram Definition
    //=============================
    TFile *f_out = TFile::Open(outputfile, "RECREATE");
    f_out->cd();
    TDirectory *histDir = f_out->mkdir("histgramtree");
    bool mass_hist=false;
    // int dummy=0;
    vector<float> ZBIN={params.Z_MASS-params.Z_WINDOW, params.Z_MASS+params.Z_WINDOW};
    vector<TString> histNames={"Lepton.PT","Lepton.Eta","Lepton.Phi","NearestZ_Mass","OtherPair_Mass"};
    vector<TString> histXLabels={"GeV"," "," ","GeV","GeV"};
    vector<int> histNBins={100,100,100,50,160};
    vector<float> histXMin={0, -3, -5, ZBIN[0], 0};
    vector<float> histXMax={200, 3, 5, ZBIN[1], 160};
     // Create histograms for each cut in the pipeline
    dummy=0;
    for (auto& step : pipeline) {
            if (step.second) { // If module is active
                for (size_t histidx=0; histidx<histNames.size(); histidx++){
                    TString histName = TString::Format("%02d_%s", dummy, histNames[histidx].Data());
                    TH1F *hist = new TH1F(histName, histName + ";" + histXLabels[histidx] + ";Events",
                        histNBins[histidx], histXMin[histidx], histXMax[histidx]);
                    hist->SetDirectory(histDir); // Associate histogram with directory
                }
            }
            dummy++;
        }
    TH2F *vsmass = new TH2F("MassPairHeatmap", "Mass Map;Pair 1 Mass (GeV);Pair 2 Mass (GeV)", 50, ZBIN[0], ZBIN[1], 160, 0, 160);
    TH1F *NotZdR = new TH1F("NotZ_dR", "dR of Not Z Pair;dR;Events", 50, 0, 6);
    TH1F *NotZdPhi = new TH1F("NotZ_dPhi", "dPhi of Not Z Pair;dPhi;Events", 50, 0, 3.5);
    NotZdR->SetDirectory(histDir); // Associate histogram with directory
    NotZdPhi->SetDirectory(histDir); // Associate histogram with directory
    vsmass->SetDirectory(histDir); // Associate histogram with directory
    // ==============================


    // Loop over events and apply cut
    Long64_t nentries = t->GetEntries();
    cout << "Processing " << nentries << " entries..." << endl;
    // variable for summary selection
    vector<int> passcut;

    int selection_counts[5] = {0, 0, 0, 0, 0};
    for (Long64_t i = 0; i < nentries; i++) {
        currentEvent.reset();
        t->GetEntry(i);
        // Check branchMap for the first event
        for (auto& step : pipeline) {
            if (step.second) { // If module is active
                step.first->process(currentEvent, params);
                if (currentEvent.PassThisCut == false) {
                    break; // Stop processing further modules
                }
                currentEvent.CutStatus[currentEvent.CurrentCut] = currentEvent.PassThisCut ? 1 : 0;
                currentEvent.CurrentCut++;
            }
        }
        // Fill the output tree
        t_out->Fill();
        // Fill histograms based on cut results
        dummy=0;
        for (auto& step : pipeline) {
            if (step.second && currentEvent.CutStatus[dummy]) { // If module is active
                for (size_t histidx=0; histidx<histNames.size(); histidx++){
                    TString histName = TString::Format("%02d_%s", dummy, histNames[histidx].Data());
                    TH1F *hist = (TH1F*)histDir->Get(histName);
                    if (hist) {
                        if (histNames[histidx]=="Lepton.PT"){
                            for (int e=0; e<currentEvent.Electron_size; e++){
                                hist->Fill(currentEvent.Electron_PT[e]);
                            }
                            for (int m=0; m<currentEvent.Muon_size; m++){
                                hist->Fill(currentEvent.Muon_PT[m]);
                            }
                        } else if (histNames[histidx]=="Lepton.Eta"){
                            for (int e=0; e<currentEvent.Electron_size; e++){
                                hist->Fill(currentEvent.Electron_Eta[e]);
                            }
                            for (int m=0; m<currentEvent.Muon_size; m++){
                                hist->Fill(currentEvent.Muon_Eta[m]);
                            }
                        } else if (histNames[histidx]=="Lepton.Phi"){
                            for (int e=0; e<currentEvent.Electron_size; e++){
                                hist->Fill(currentEvent.Electron_Phi[e]);
                            }
                            for (int m=0; m<currentEvent.Muon_size; m++){
                                hist->Fill(currentEvent.Muon_Phi[m]);
                            }
                        } else if (histNames[histidx]=="NearestZ_Mass"){
                            hist->Fill(currentEvent.NearestZ_Mass);
                        } else if (histNames[histidx]=="OtherPair_Mass"){
                            hist->Fill(currentEvent.OtherPair_Mass);
                        }
                    }
                }
            }
            dummy++;
        }
        // Fill 2D mass histogram
        if (currentEvent.CutStatus[4]==1){ // Passed Z_Window cut
            vsmass->Fill(currentEvent.NearestZ_Mass, currentEvent.OtherPair_Mass);
            NotZdR->Fill(currentEvent.NotZ_dR);
            NotZdPhi->Fill(currentEvent.NotZ_dPhi);
        }

        // Find index of 1 in CutStatus
        for (int i=0; i < 10; i++){
            if (currentEvent.CutStatus[i]==1){
                selection_counts[i]=selection_counts[i]+1;
            }
        }
        // report processed status every 100k events
        if (i % 100000 == 0) {
            cout << " Processed " << i << " events..." << endl;
        }
    }
    // ==============================
    // Cleanup
    // ==============================
    for (auto& step : pipeline) {
        delete step.first;
    }
    // Close input file
    f->Close();
    // ==============================
    // Summary of selection
    // ==============================

    cout << "Summary of selection:" << endl;
    cout << " Total events processed: " << nentries << endl;
    cout << " Events passing each cut stage:" << endl;
    cout << "  After Lepton PT cut: " << selection_counts[0] << endl;
    cout << "  After Final State 4 Leptons cut: " << selection_counts[1] << endl;
    cout << "  After Lepton Odd cut: " << selection_counts[2] << endl;
    cout << "  After Charge Violation cut: " << selection_counts[3] << endl;
    cout << "  After Z Window cut: " << selection_counts[4] << endl;

    // test loop overhead time without calculation
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    cout << "Loop overhead time: " << elapsed.count() << " seconds" << endl;
    //==============================
    // Write Tree Output
    //==============================
    tfile_out->cd();
    t_out->Write();
    tfile_out->Close();

    //==============================
    // Write output histogram to file
    //==============================
    f_out->cd();
    histDir->Write(); // Write all histograms in the directory
    // Clean up
    cout << "Done. Output written to: " << outputfile << endl;
    f_out->Close();
    // t_out->Close();
    //==============================
    gApplication->Terminate(); // Clean ROOT termination
}