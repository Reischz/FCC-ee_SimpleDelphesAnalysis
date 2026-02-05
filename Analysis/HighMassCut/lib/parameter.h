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
    static constexpr float LFV_MinPT = 30.0; // GeV
    static constexpr float Z_Treshold_offShell = 5.0; // GeV
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
    vector<TString> Z_PairIndexes,NotZ_PairIndexes;
    float NotZ_dR;
    float NotZ_dPhi;
    float NotZ_EleMET_dPhi;
    float NotZ_MuMET_dPhi;

    float Particle_PT[100]; // assuming max 100 particles
    float Particle_Eta[100];
    float Particle_Phi[100];
    int Particle_PID[100];
    int Particle_M1[100];
    int Particle_M2[100];
    int Particle_D1[100];
    int Particle_D2[100];
    int Particle_Charge[100];
    int Particle_Status[100];
    int Particle_size;

    bool Matching_SingleLepSide;
    bool Matching_ThreeLepSide;
    bool Matching_Perfect;
    bool Matching_OppositeLep;

    float SingleLep_dRtoGen, ThreeLep_dRtoGen;
    vector<float> FreeAllLep_dRtoGen;
    vector<bool> FreeAllLep_MatchStatus;

    float SFSC_GendR;
    float SFSC_RecodR;

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
        Z_PairIndexes.clear();
        NotZ_PairIndexes.clear();
        NotZ_EleMET_dPhi = -1.0;
        NotZ_MuMET_dPhi = -1.0;
        NotZ_dR = -1.0;
        NotZ_dPhi = -1.0;
        for (int i = 0; i < 10; ++i) {
            CutStatus[i] = 0;
        }

        for (int i = 0; i < 4; ++i) {
            Electron_PT[i] = 0.0;
            Electron_Eta[i] = 0.0;
            Electron_Phi[i] = 0.0;
            Electron_Charge[i] = 0;
            Muon_PT[i] = 0.0;
            Muon_Eta[i] = 0.0;
            Muon_Phi[i] = 0.0;
            Muon_Charge[i] = 0;
        }

        Matching_SingleLepSide = false;
        Matching_ThreeLepSide = false;
        Matching_Perfect = false;
        Matching_OppositeLep = false;

        SFSC_GendR = -1.0;
        SFSC_RecodR = -1.0;
        SingleLep_dRtoGen = -1.0;
        ThreeLep_dRtoGen = -1.0;
        FreeAllLep_dRtoGen.clear();
        FreeAllLep_MatchStatus.clear();
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

TString IdentifyOriginLepton(int pid, EventContext &data, int index) {
    // Identify whether the lepton is from Z, W, or other sources
    int mother1 = data.Particle_M1[index];
    int mother2 = data.Particle_M2[index];
    
    int mother1_pid = data.Particle_PID[mother1];
    int mother2_pid = data.Particle_PID[mother2];

    // if ((mother1 != -1) && (mother2 != -1)) {
    //     return continue;
    // }
    // cout << " Identifying Origin for PID: " << pid << ", Index: " << index << ", Mother1 PID: " << mother1_pid << ", Mother2 PID: " << mother2_pid << endl;
    if ((mother1_pid == pid) || (mother2_pid == pid)) {
        // cout << " Tracing back to mother particle." << endl;
        return IdentifyOriginLepton(pid, data, mother1_pid == pid ? mother1 : mother2);
    }
    else if ((mother1_pid == 23) || (mother2_pid == 23)) {
        return "Z";
    } else if ((mother1_pid == 25) || (mother2_pid == 25)) {
        return "Higgs";
    } else {
        return "Other";
    }
}