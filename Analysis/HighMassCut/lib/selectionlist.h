#pragma once
#include "parameter.h"
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
        bool isPairedLepton=false;
};

// ==========================================
// Analysis Module
// ==========================================
class NonSelection : public AnalysisModule {
    public:
        NonSelection() : AnalysisModule("NonSelection") {}

        void process(EventContext &data, const defaultParameters &params) override {
            return;
        }
};
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
        Z_Window() : AnalysisModule("Z_Window") {
            isPairedLepton=true;
        }

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
        NotZ_dR() : AnalysisModule("NotZ_dR") {
            isPairedLepton=true;
        }

        void process(EventContext &data, const defaultParameters &params) override {
            if ((!data.CutStatus[data.CurrentCut-1]) && data.CurrentCut>0){
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
        NotZ_MET_dPhi() : AnalysisModule("NotZ_MET_dPhi") {
            isPairedLepton=true;
        }

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
        NotZ_MassThreshold() : AnalysisModule("NotZ_MassThreshold") {
            isPairedLepton=true;
        }

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

class HighMass_PT : public AnalysisModule {
    public:
        HighMass_PT() : AnalysisModule("HighMass_PT") {
            isPairedLepton=true;
        }
        void process(EventContext &data, const defaultParameters &params) override {
            // =======================Defined Varibles==========================
            return;
        }
};

class PairSelection_offshell : public AnalysisModule {
    public:
        PairSelection_offshell() : AnalysisModule("PairSelection_offshell") {
            isPairedLepton=true;
        }
        void process(EventContext &data, const defaultParameters &params) override {
            // ========================================================Defined Varibles==========================
            bool EMore= (data.Electron_size > data.Muon_size);
            TString ThreeLepFlavour=  EMore ? "E" : "Mu";
            TString SingleLepFlavour= EMore ? "Mu" : "E";
            float* ThreeLep_PT=     EMore ? data.Electron_PT : data.Muon_PT;
            float* ThreeLep_Eta=    EMore ? data.Electron_Eta : data.Muon_Eta;
            float* ThreeLep_Phi=    EMore ? data.Electron_Phi : data.Muon_Phi;
            int* ThreeLep_Charge= EMore ? data.Electron_Charge : data.Muon_Charge;
            float* SingleLep_PT=    EMore ? data.Muon_PT : data.Electron_PT;
            float* SingleLep_Eta=   EMore ? data.Muon_Eta : data.Electron_Eta;
            float* SingleLep_Phi=   EMore ? data.Muon_Phi : data.Electron_Phi;
            int* SingleLep_Charge=EMore ? data.Muon_Charge : data.Electron_Charge;
            const float& SingleLep_MASS=   EMore ? params.Muon_MASS : params.Electron_MASS;
            // ========================================================Processing==========================
            TLorentzVector SgleVec, HCandLep1Vec, HCandLep2Vec;
            SgleVec.SetPtEtaPhiM(
                SingleLep_PT[0],
                SingleLep_Eta[0],
                SingleLep_Phi[0],
                SingleLep_MASS
            );
            vector<int> NotHLepIdxLst={};
            for (int i=0; i<3; i++) ((ThreeLep_Charge[i]==SingleLep_Charge[0]) ? NotHLepIdxLst.push_back(i) : void());
            // Not Passing Condition 1: Charge Violation
            if (NotHLepIdxLst.size()!=1){
                data.PassThisCut = false;
                return;}
            throw runtime_error("Charge Violation Check is Compromised");
            return;
        }
};

class Verify_Generator : public AnalysisModule {
    public:
        Verify_Generator() : AnalysisModule("Verify_Generator") {}
        void process(EventContext &data, const defaultParameters &params) override {
            return;
        }
};


//===================================================================================================
// Lagacy
//===================================================================================================
class PairSelection_offshell_VO : public AnalysisModule {
    public:
        PairSelection_offshell_VO() : AnalysisModule("PairSelection_offshell_VO") {
            isPairedLepton=true;
        }

        void process(EventContext &data, const defaultParameters &params) override {
            // Placeholder for future implementation
            vector<float> HiggsCandidateMass;
            vector<float> ZCandidateMass;
            TLorentzVector lepton1Vector, lepton2Vector, combinedVector;
            vector<int> ZindexesSum;
            vector<float> dRvalues;
            for (size_t i = 0; i < data.Electron_size; i++) {
                for (size_t j = 0; j < data.Muon_size; j++) {
                    if ((data.Electron_Charge[i] * data.Muon_Charge[j]) > 0) {
                        continue; // Same charge, skip
                    }
                    else if ((data.Muon_PT[j]<params.LFV_MinPT) || (data.Electron_PT[i]<params.LFV_MinPT)){
                        continue; // Below PT cut, skip
                    }
                    // Use TLorentzVector to calculate invariant mass
                    lepton1Vector.SetPtEtaPhiM(data.Electron_PT[i], data.Electron_Eta[i], data.Electron_Phi[i], params.Electron_MASS);
                    lepton2Vector.SetPtEtaPhiM(data.Muon_PT[j], data.Muon_Eta[j], data.Muon_Phi[j], params.Muon_MASS);
                    float dR=lepton1Vector.DeltaR(lepton2Vector);
                    if (dR<2){
                        continue; // dR cut, skip
                    }
                    dRvalues.push_back(dR);
                    combinedVector = lepton1Vector + lepton2Vector;
                    float mass = combinedVector.M();

                    HiggsCandidateMass.push_back(mass);

                    TLorentzVector Z_Candidate1Vec, Z_Candidate2Vec;
                    vector<int> allindexes={0,1,2};
                    vector<int> thisindexes;
                    if (data.Electron_size>data.Muon_size) {
                        // Calculate Z candidate mass from remaining leptons
                        for (int idx : allindexes) {
                            if (idx != i) {
                                thisindexes.push_back(idx);
                            }
                        }
                        Z_Candidate1Vec.SetPtEtaPhiM(data.Electron_PT[thisindexes[0]], data.Electron_Eta[thisindexes[0]], data.Electron_Phi[thisindexes[0]], params.Electron_MASS);
                        Z_Candidate2Vec.SetPtEtaPhiM(data.Electron_PT[thisindexes[1]], data.Electron_Eta[thisindexes[1]], data.Electron_Phi[thisindexes[1]], params.Electron_MASS);

                    } else {
                        // Calculate Z candidate mass from remaining leptons
                        for (int idx : allindexes) {
                            if (idx != j) {
                                thisindexes.push_back(idx);
                            }
                        }
                        Z_Candidate1Vec.SetPtEtaPhiM(data.Muon_PT[thisindexes[0]], data.Muon_Eta[thisindexes[0]], data.Muon_Phi[thisindexes[0]], params.Muon_MASS);
                        Z_Candidate2Vec.SetPtEtaPhiM(data.Muon_PT[thisindexes[1]], data.Muon_Eta[thisindexes[1]], data.Muon_Phi[thisindexes[1]], params.Muon_MASS);

                    }
                    float ThisZMass= (Z_Candidate1Vec + Z_Candidate2Vec).M();
                    if ((ThisZMass > params.Z_MASS) || (ThisZMass < params.Z_Treshold_offShell)) {
                        continue; // Z mass threshold cut, skip
                    }
                    ZCandidateMass.push_back(ThisZMass);
                    ZindexesSum.push_back(thisindexes[0] + thisindexes[1]);
                }
            }
            // cout << "Found candidate pair: Higgs Mass "<< endl;
            int size_count=ZCandidateMass.size();
            if (size_count == 0) {
                data.PassThisCut = false;
            } else if (size_count == 1) {
                // cout << "data single begin" << endl;
                // Select the candidate with Higgs mass closest to 125 GeV
                data.OtherPair_Mass = HiggsCandidateMass[0];
                data.NearestZ_Mass = ZCandidateMass[0];
                // cout << "data single end" << endl;
            }
            else {
                // cout << "data pair begin" << endl;
                float highHMass = 0.0;
                int highIndex = -1;
                for (size_t idx = 0; idx < size_count; idx++) {
                    if (HiggsCandidateMass[idx] > highHMass) {
                        highHMass = HiggsCandidateMass[idx];
                        highIndex = idx;
                    }
                }
                data.OtherPair_Mass = HiggsCandidateMass[highIndex];
                data.NearestZ_Mass = ZCandidateMass[highIndex];
                data.Z_PairIndexSum = ZindexesSum[highIndex];
                data.NotZ_dR = dRvalues[highIndex];
                // cout << "pair end" << endl;
            }
            return;
        }
};

class Verify_Generator_VO : public AnalysisModule {
    public:
        Verify_Generator_VO() : AnalysisModule("Verify_Generator_VO") {}

        void process(EventContext &data, const defaultParameters &params) override {
            // Placeholder for future implementation
            // if (data.Previous == 0) {
            //     data.PassThisCut = false;
            // }
            vector<int> FinalstateIndexes;
            vector<TString> FinalstateMother;
            for (auto ParContentNum=0; ParContentNum<data.Particle_size; ParContentNum++){
                if (data.Particle_PID[ParContentNum]==22 || data.Particle_Status[ParContentNum]!=1){
                    continue;
                }
                else if (abs(data.Particle_PID[ParContentNum])!=11 && abs(data.Particle_PID[ParContentNum])!=13){
                    continue;
                }
                else if (data.Particle_M1[ParContentNum]==-1 && data.Particle_M2[ParContentNum]==-1){
                    continue;
                }
                FinalstateMother.push_back(IdentifyOriginLepton(data.Particle_PID[ParContentNum], data, ParContentNum));
                FinalstateIndexes.push_back(ParContentNum);
            }
            TLorentzVector generatorLepton, detectorLepton;
            TLorentzVector OppositeGeneratorLepton;
            bool ElectronDominated= (data.Electron_size > data.Muon_size);
            int LFVIndexin3lepside=3-data.Z_PairIndexSum;
            bool MatchedSingleLepSide=false;
            bool MatchedThreeLepSide=false;
            bool MatchedOppositeLep=false;
            bool PerfectMatch=false;
            float dR;
            int thisFlavor;
            float thisFlavorMass;
            float dR_Opposite;
            for (size_t i=0; i<FinalstateMother.size(); i++){
                if (FinalstateMother[i]=="Other"){
                    continue;
                }
                else if (FinalstateMother[i]=="Higgs"){
                    thisFlavor=data.Particle_PID[FinalstateIndexes[i]];
                    thisFlavorMass=(abs(thisFlavor)==11)? params.Electron_MASS : params.Muon_MASS;
                    generatorLepton.SetPtEtaPhiM(
                        data.Particle_PT[FinalstateIndexes[i]], 
                        data.Particle_Eta[FinalstateIndexes[i]], 
                        data.Particle_Phi[FinalstateIndexes[i]], 
                        thisFlavorMass
                    );
                    // Match to detector level leptons
                    if (ElectronDominated && (abs(thisFlavor) == 13)){
                        // Match to Electron
                        detectorLepton.SetPtEtaPhiM(
                            data.Electron_PT[LFVIndexin3lepside],
                            data.Electron_Eta[LFVIndexin3lepside],
                            data.Electron_Phi[LFVIndexin3lepside],
                            params.Electron_MASS
                        );
                        dR=generatorLepton.DeltaR(detectorLepton);
                        // Identify Opposite Candidate
                        for (int k=0; k<FinalstateIndexes.size(); k++){
                            if ((k!=i) && (abs(data.Particle_PID[FinalstateIndexes[k]])==11)){
                                OppositeGeneratorLepton.SetPtEtaPhiM(
                                    data.Particle_PT[FinalstateIndexes[k]], 
                                    data.Particle_Eta[FinalstateIndexes[k]], 
                                    data.Particle_Phi[FinalstateIndexes[k]], 
                                    params.Electron_MASS
                                );
                                dR_Opposite=OppositeGeneratorLepton.DeltaR(detectorLepton);
                                if (dR_Opposite<0.1 || dR_Opposite==0.1){
                                    MatchedOppositeLep=true;
                                }
                            }
                        }
                        if (dR<0.1 || dR==0.1){
                            MatchedThreeLepSide=true;
                            continue;
                        }
                    } else if (!ElectronDominated && (abs(thisFlavor) == 11)){
                        // Match to Muon
                        detectorLepton.SetPtEtaPhiM(
                            data.Muon_PT[LFVIndexin3lepside],
                            data.Muon_Eta[LFVIndexin3lepside],
                            data.Muon_Phi[LFVIndexin3lepside],
                            params.Muon_MASS
                        );
                        dR=generatorLepton.DeltaR(detectorLepton);
                        if (dR<0.1 || dR==0.1){
                            MatchedThreeLepSide=true;
                            continue;
                        }
                    }
                    else if (ElectronDominated && (abs(thisFlavor) == 11)){
                        // Match to Muon
                        detectorLepton.SetPtEtaPhiM(
                            data.Muon_PT[0],
                            data.Muon_Eta[0],
                            data.Muon_Phi[0],
                            params.Muon_MASS
                        );
                        dR=generatorLepton.DeltaR(detectorLepton);
                        // Identify Opposite Candidate
                        for (int k=0; k<FinalstateIndexes.size(); k++){
                            if ((k==i) || (abs(data.Particle_PID[FinalstateIndexes[k]])!=13)){
                                continue;
                            }
                            else {
                                OppositeGeneratorLepton.SetPtEtaPhiM(
                                    data.Particle_PT[FinalstateIndexes[k]], 
                                    data.Particle_Eta[FinalstateIndexes[k]], 
                                    data.Particle_Phi[FinalstateIndexes[k]], 
                                    params.Muon_MASS
                                );
                                float dR_Opposite=OppositeGeneratorLepton.DeltaR(detectorLepton);
                                if (dR_Opposite<0.1 || dR_Opposite==0.1){
                                    MatchedOppositeLep=true;
                                }
                            }
                        }
                        if (dR<0.1 || dR==0.1){
                            MatchedSingleLepSide=true;
                            continue;
                        }
                    }
                    else if (!ElectronDominated && (abs(thisFlavor) == 13)){
                        // Match to Electron
                        detectorLepton.SetPtEtaPhiM(
                            data.Electron_PT[0],
                            data.Electron_Eta[0],
                            data.Electron_Phi[0],
                            params.Electron_MASS
                        );
                        dR=generatorLepton.DeltaR(detectorLepton);
                        if (dR<0.1 || dR==0.1){
                            MatchedSingleLepSide=true;
                            continue;
                        }
                    }
                }
            }
            if (MatchedSingleLepSide && MatchedThreeLepSide){
                PerfectMatch=true;
            }
            data.Matching_SingleLepSide=MatchedSingleLepSide;
            data.Matching_ThreeLepSide=MatchedThreeLepSide;
            data.Matching_OppositeLep=MatchedOppositeLep;
            data.Matching_Perfect=PerfectMatch;
            // To Verify if the criteria of using dR is relaiable, utilizing SFSC_GendR and SFSC_RecodR
            // Find final states particle that is SFSC
            TLorentzVector SFSC1_GenVec, SFSC2_GenVec,SFSC1_RecVec, SFSC2_RecVec;
            float ThisPairParticlesMass;
            int GenFinalStateSize=FinalstateIndexes.size();
            for (int j=0; j<GenFinalStateSize;j++){
                for (int l=0; l<GenFinalStateSize;l++){
                    if (j==l){
                        continue;
                    }
                    else if (data.Particle_PID[FinalstateIndexes[l]]==data.Particle_PID[FinalstateIndexes[j]]){
                        ThisPairParticlesMass= (abs(data.Particle_PID[FinalstateIndexes[j]])==11)? params.Electron_MASS : params.Muon_MASS;
                        SFSC1_GenVec.SetPtEtaPhiM(
                            data.Particle_PT[FinalstateIndexes[j]], 
                            data.Particle_Eta[FinalstateIndexes[j]], 
                            data.Particle_Phi[FinalstateIndexes[j]], 
                            ThisPairParticlesMass
                        );
                        SFSC2_GenVec.SetPtEtaPhiM(
                            data.Particle_PT[FinalstateIndexes[l]], 
                            data.Particle_Eta[FinalstateIndexes[l]], 
                            data.Particle_Phi[FinalstateIndexes[l]], 
                            ThisPairParticlesMass
                        );
                        break;
                    }
                }
            }
            data.SFSC_GendR=SFSC1_GenVec.DeltaR(SFSC2_GenVec);
            // Find detector level particles that is SFSC
            // We know LFV lepton index in 3 lepton side is LFVIndexin3lepside
            vector<int> allindexes={LFVIndexin3lepside};
            // 3E +1Muon
            if (data.Electron_size>data.Muon_size){
                for (int m=0; m< data.Electron_size; m++){
                    if ((m!=LFVIndexin3lepside) && (data.Electron_Charge[m]==data.Electron_Charge[LFVIndexin3lepside])){
                        allindexes.push_back(m);
                        break;
                    }
                }
                if (allindexes.size()<2){
                    // Safety check
                    return;
                }
                SFSC1_RecVec.SetPtEtaPhiM(
                    data.Electron_PT[allindexes[0]],
                    data.Electron_Eta[allindexes[0]],
                    data.Electron_Phi[allindexes[0]],
                    params.Electron_MASS
                );
                SFSC2_RecVec.SetPtEtaPhiM(
                    data.Electron_PT[allindexes[1]],
                    data.Electron_Eta[allindexes[1]],
                    data.Electron_Phi[allindexes[1]],
                    params.Electron_MASS
                );
            }
            else{
                for (int m=0; m< data.Muon_size; m++){
                    if ((m!=LFVIndexin3lepside) && (data.Muon_Charge[m]==data.Muon_Charge[LFVIndexin3lepside])){
                        allindexes.push_back(m);
                        break;
                    }
                }
                if (allindexes.size()<2){
                    // Safety check
                    return;
                }
                SFSC1_RecVec.SetPtEtaPhiM(
                    data.Muon_PT[allindexes[0]],
                    data.Muon_Eta[allindexes[0]],
                    data.Muon_Phi[allindexes[0]],
                    params.Muon_MASS
                );
                SFSC2_RecVec.SetPtEtaPhiM(
                    data.Muon_PT[allindexes[1]],
                    data.Muon_Eta[allindexes[1]],
                    data.Muon_Phi[allindexes[1]],
                    params.Muon_MASS
                );
            }
            data.SFSC_RecodR=SFSC1_RecVec.DeltaR(SFSC2_RecVec);
            return;
        }
};