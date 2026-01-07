#pragma once
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
    // bool readyformasshist;

    // Constructor to reset values per loop
    void reset() {
        Event_size = 0;
        Electron_size = 0;
        Muon_size = 0;
        CurrentCut = 0;
        PassThisCut = true;
        readyformasshist = false;
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