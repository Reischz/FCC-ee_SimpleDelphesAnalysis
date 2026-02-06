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

    // High Higgs critia info
    float HH_NotZPair_Mass;
    float HH_Ant_NotZPair_Mass;
    float HH_ZPair_Mass;
    float HH_Ant_ZPair_Mass;
    int Z_PairIndexSum;
    vector<TString> HH_ZPairIndexes,HH_NotZPairIndexes;
    float HH_NotZ_dR;
    float HH_NotZ_dPhi;
    float NotZ_EleMET_dPhi;
    float NotZ_MuMET_dPhi;

    // Z critia matching info
    float ZC_ZPairdR;
    float ZC_ZPairMass;
    float ZC_Ant_ZPairMass;
    float ZC_NotZPairMass;
    float ZC_Ant_NotZPairMass;
    float ZC_NotZdR;
    vector<TString> ZC_ZPairIndexes,ZC_NotZPairIndexes;

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
    float Particle_Mass[100];
    int Particle_size;

    bool Matching_SingleLepSide;
    bool Matching_ThreeLepSide;
    bool Matching_Perfect;
    bool Matching_OppositeLep;

    float Gen_Higgs_Mass;
    float Gen_Z_Mass;
    float Gen_DirectHMass;
    float Gen_DirectZMass;
    float GentoReco_HMass;
    float GentoReco_ZMass;

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

        // Reset High Higgs critia info
        HH_ZPair_Mass = -1.0;
        HH_NotZPair_Mass = -1.0;
        Z_PairIndexSum = -1;
        HH_ZPairIndexes.clear();
        HH_NotZPairIndexes.clear();
        NotZ_EleMET_dPhi = -1.0;
        NotZ_MuMET_dPhi = -1.0;
        HH_NotZ_dR = -1.0;
        HH_NotZ_dPhi = -1.0;

        // Reset Z critia matching info
        ZC_ZPairdR = -1.0;
        ZC_ZPairMass = -1.0;
        ZC_NotZPairMass = -1.0;
        ZC_NotZdR = -1.0;
        ZC_ZPairIndexes.clear();
        ZC_NotZPairIndexes.clear();

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

        Gen_Higgs_Mass = -1.0;
        Gen_Z_Mass = -1.0;
        Gen_DirectHMass = -1.0;
        Gen_DirectZMass = -1.0;
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