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