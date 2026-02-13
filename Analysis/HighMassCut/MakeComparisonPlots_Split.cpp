#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1.h>
#include <THStack.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TSystemDirectory.h>
#include <TList.h>
#include <iostream>
#include <iomanip> // Added for table formatting
#include <vector>
#include <map>

using namespace std;

// =========================================================================
// 1. REUSABLE HISTOGRAM MANAGER
// =========================================================================
struct HistManager {
    TDirectory* dir = nullptr;
    void SetDirectory(TDirectory* d) { dir = d; }
    void WriteStack(THStack* stack, TString name) { if (dir) dir->cd(); stack->Write(name); }
    void WriteCanvas(TCanvas* c) { if (dir) dir->cd(); c->Write(); }
};

// =========================================================================
// 2. HELPER FUNCTIONS
// =========================================================================

TString GetFinalCutBranchName(TTree* t) {
    TObjArray* branches = t->GetListOfBranches();
    TString bestBranch = "";
    int maxIndex = -1;
    for (int i = 0; i < branches->GetEntries(); i++) {
        TString bName = branches->At(i)->GetName();
        if (bName.BeginsWith("Status_")) {
            TString numStr = bName(7, 2);
            if (numStr.IsDigit()) {
                int index = numStr.Atoi();
                if (index > maxIndex) { maxIndex = index; bestBranch = bName; }
            }
        }
    }
    return bestBranch;
}

bool IsAdditionalTree(TString filename) {
    return filename.EndsWith("AdditionalTree.root") && !filename.BeginsWith(".");
}

// Styling: Signals = Solid, Backgrounds = Dashed/Dotted
void StyleHistogram(TH1F* h, int index, TString name) {
    bool isSignal = name.Contains("HLFV");
    
    int colors[] = {kRed+1, kBlue+1, kGreen+2, kMagenta+1, kOrange+1, kCyan+1, kViolet, kBlack, kGray+2};
    int colorIdx = index % 9;
    
    h->SetLineColor(colors[colorIdx]);
    h->SetLineWidth(2);
    
    if (isSignal) {
        h->SetLineStyle(kSolid);
        h->SetFillColorAlpha(colors[colorIdx], 0.05); // Very light fill for signal
    } else {
        h->SetLineStyle(name.Contains("OnS") ? kDashed : kDotted); 
        h->SetFillColorAlpha(colors[colorIdx], 0.0); // No fill for BG
    }
    
    h->SetStats(0);
    h->SetTitle(name);
}

// =========================================================================
// 3. TABLE GENERATION FUNCTIONS
// =========================================================================

// Helper to quickly count events passing the final cut + mass window
double GetYieldForTable(TString filepath, double targetMass, double massWindow) {
    TFile* f = TFile::Open(filepath);
    if (!f || f->IsZombie()) return 0.0; // Return 0 if file is missing

    TTree* t = (TTree*)f->Get("Selection Results");
    if (!t) { f->Close(); return 0.0; }

    TString statusBranch = GetFinalCutBranchName(t);
    if (statusBranch == "") { f->Close(); return 0.0; }

    // Build the string: "Status_XX == 1 && abs(HH_NotZPair_Mass - targetMass) < 5.0"
    TString cutString = TString::Format("%s == 1 && abs(HH_NotZPair_Mass - %f) < %f", 
                                        statusBranch.Data(), targetMass, massWindow);

    double count = t->GetEntries(cutString);
    f->Close();
    return count;
}

void PrintYieldTable(const map<TString, double>& bg_xsec) {
    vector<int> masses = {110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160};
    double massWindow = 5.0; // +/- 5 GeV

    cout << "\n==============================================================================================================" << endl;
    cout << "                         EXPECTED YIELDS AT 1 ab^{-1} (MASS WINDOW +/- 5 GeV)                                 " << endl;
    cout << "==============================================================================================================" << endl;
    cout << setw(8) << "Mass" << " | " << setw(15) << "Signal Yield" << " | ";
    for (auto const& [bg, xsec] : bg_xsec) {
        cout << setw(12) << bg << " | ";
    }
    cout << setw(15) << "Total BG" << endl;
    cout << "--------------------------------------------------------------------------------------------------------------" << endl;
    
    for (int m : masses) {
        // Determine correct directory based on mass range
        TString dir = (m <= 145) ? "SelectionResults/OnShellCut/" : "SelectionResults/CombineAll/";
        
        // 1. Calculate Signal Yield
        TString sigFile = dir + TString::Format("HLFV_%dGeV_AdditionalTree.root", m);
        double sigYield = GetYieldForTable(sigFile, m, massWindow) * 1e-6;
        
        cout << setw(4) << m << " GeV | " << setw(15) << scientific << setprecision(3) << sigYield << " | ";
        
        // 2. Calculate Background Yields
        double totalBg = 0.0;
        for (auto const& [bg, xsec] : bg_xsec) {
            TString bgFile = dir + bg + "_AdditionalTree.root";
            double bgYield = GetYieldForTable(bgFile, m, massWindow) * xsec;
            totalBg += bgYield;
            cout << setw(12) << scientific << setprecision(3) << bgYield << " | ";
        }
        cout << setw(15) << scientific << setprecision(3) << totalBg << endl;
    }
    cout << "==============================================================================================================\n" << endl;
}


// =========================================================================
// 4. CORE PROCESSING FUNCTION (Plots)
// =========================================================================
void ProcessDirectory(
    TString inputDir, TString suffixLabel, int minSignalMass, int maxSignalMass,
    THStack* specificStackZ, THStack* specificStackNotZ, 
    THStack* combinedStackZ, THStack* combinedStackNotZ,
    TLegend* specificLeg, TLegend* combinedLeg,
    int& globalIndex, const map<TString, double>& bg_xsec
) {
    TSystemDirectory dir(inputDir, inputDir);
    TList *files = dir.GetListOfFiles();
    
    if (!files) return;

    files->Sort();
    TIter next(files);
    TSystemFile *file;

    cout << ">>> Processing Directory: " << inputDir << " (Suffix: " << suffixLabel << ")" << endl;

    while ((file = (TSystemFile*)next())) {
        TString fname = file->GetName();
        if (!IsAdditionalTree(fname)) continue;

        TString baseName = fname;
        baseName.ReplaceAll("_AdditionalTree.root", "");

        bool isSignal = baseName.Contains("HLFV");
        if (isSignal) {
            TString massStr = baseName;
            massStr.ReplaceAll("HLFV_", "");
            massStr.ReplaceAll("GeV", "");
            int mass = massStr.Atoi();
            if (mass < minSignalMass || mass > maxSignalMass) continue;
        }
        
        double weight = 1.0;
        if (isSignal) {
            weight = 1e-6; 
        } else {
            if (bg_xsec.count(baseName)) weight = bg_xsec.at(baseName);
        }

        TString uniqueName = baseName;
        if (!isSignal) uniqueName += suffixLabel;

        cout << "   -> Adding to Plots: " << uniqueName << endl;

        TFile *fIn = TFile::Open(inputDir + fname);
        if (!fIn || fIn->IsZombie()) continue;

        TTree *t = (TTree*)fIn->Get("Selection Results");
        if (!t) { fIn->Close(); continue; }

        TString cutBranchName = GetFinalCutBranchName(t);
        if (cutBranchName == "") { fIn->Close(); continue; }
        
        int passStatus = 0;
        float HH_ZPair_Mass = 0;
        float HH_NotZPair_Mass = 0;

        t->SetBranchAddress(cutBranchName, &passStatus);
        t->SetBranchAddress("HH_ZPair_Mass", &HH_ZPair_Mass);
        t->SetBranchAddress("HH_NotZPair_Mass", &HH_NotZPair_Mass);

        TH1F *h_Z = new TH1F(uniqueName + "_Z", uniqueName, 50, 0, 250);
        TH1F *h_NotZ = new TH1F(uniqueName + "_NotZ", uniqueName, 50, 0, 250);
        
        h_Z->Sumw2();
        h_NotZ->Sumw2();
        h_Z->SetDirectory(0);
        h_NotZ->SetDirectory(0);

        Long64_t nentries = t->GetEntries();
        for (Long64_t i = 0; i < nentries; i++) {
            t->GetEntry(i);
            if (passStatus == 1) {
                h_Z->Fill(HH_ZPair_Mass);
                h_NotZ->Fill(HH_NotZPair_Mass);
            }
        }

        h_Z->Scale(weight);
        h_NotZ->Scale(weight);

        StyleHistogram(h_Z, globalIndex, uniqueName);
        StyleHistogram(h_NotZ, globalIndex, uniqueName);
        globalIndex++;

        specificStackZ->Add(h_Z);
        specificStackNotZ->Add(h_NotZ);
        specificLeg->AddEntry(h_Z, uniqueName, "l");

        TH1F* h_Z_Comb = (TH1F*)h_Z->Clone(uniqueName + "_Z_Comb");
        TH1F* h_NotZ_Comb = (TH1F*)h_NotZ->Clone(uniqueName + "_NotZ_Comb");
        h_Z_Comb->SetDirectory(0);
        h_NotZ_Comb->SetDirectory(0);

        combinedStackZ->Add(h_Z_Comb);
        combinedStackNotZ->Add(h_NotZ_Comb);
        combinedLeg->AddEntry(h_Z_Comb, uniqueName, "l");

        fIn->Close();
    }
}

void ExportYieldsForCombine(const map<TString, double>& bg_xsec) {
    // We only process the masses listed in your bash script
    vector<int> masses = {110, 115, 120, 125, 130, 135, 140, 145};
    double massWindow = 5.0; 

    // Create the UpperLimit directory if it doesn't exist
    gSystem->mkdir("UpperLimit", kTRUE);
    
    // Open output file
    ofstream outFile("UpperLimit/yields_database.txt");
    if (!outFile.is_open()) {
        cerr << "Failed to open UpperLimit/yields_database.txt for writing!" << endl;
        return;
    }

    cout << "Exporting yields to UpperLimit/yields_database.txt..." << endl;

    for (int m : masses) {
        // Since these are 110-145, they are in the OnShell directory
        TString dir = "SelectionResults/OnShellCut/"; 
        
        // 1. Calculate Signal Yield (XXXX)
        TString sigFile = dir + TString::Format("HLFV_%dGeV_AdditionalTree.root", m);
        double sigYield = GetYieldForTable(sigFile, m, massWindow) * 1e-6;
        if (sigYield <= 0.0) sigYield = 1e-12; // Prevent Combine crash

        // 2. Calculate ZWW4l Yield (ZZZZ)
        double zww4l_yield = GetYieldForTable(dir + "ZWW4l_AdditionalTree.root", m, massWindow) * bg_xsec.at("ZWW4l");
        if (zww4l_yield <= 0.0) zww4l_yield = 1e-12; // Prevent Combine crash

        // 3. Calculate HZ4l Yield (YYYY) - Summing the rest
        double hz4l_yield = 0.0;
        hz4l_yield += GetYieldForTable(dir + "ZHTaTa_AdditionalTree.root", m, massWindow) * bg_xsec.at("ZHTaTa");
        hz4l_yield += GetYieldForTable(dir + "ZHWW_AdditionalTree.root", m, massWindow) * bg_xsec.at("ZHWW");
        hz4l_yield += GetYieldForTable(dir + "ZZTaTa_AdditionalTree.root", m, massWindow) * bg_xsec.at("ZZTaTa");
        if (hz4l_yield <= 0.0) hz4l_yield = 1e-12; // Prevent Combine crash

        // Write row: Mass Signal HZ4l ZWW4l
        // Using scientific notation ensures precision
        outFile << m << " " 
                << scientific << setprecision(6) << sigYield << " " 
                << scientific << setprecision(6) << hz4l_yield << " " 
                << scientific << setprecision(6) << zww4l_yield << "\n";
    }
    
    outFile.close();
    cout << "Export complete." << endl;
}

// =========================================================================
// 5. MAIN MACRO
// =========================================================================
void MakeComparisonPlots_Split(TString outputFile = "Comparison_Distributions_Full.root") { 
    
    map<TString, double> bg_xsec = {
        {"ZHTaTa", 2.19e-5},
        {"ZHWW",   7.84e-5},
        {"ZZTaTa", 1.52e-4},
        {"ZWW4l",  2.691e-6}
    };

    TFile *fOut = TFile::Open(outputFile, "RECREATE");
    HistManager hm;
    hm.SetDirectory(fOut);

    // --- A. Define Stacks ---
    THStack *hs_OnS_Z = new THStack("hs_OnS_Z", "On-Shell: HH Z-Pair Mass;Mass [GeV];Events");
    THStack *hs_OnS_NotZ = new THStack("hs_OnS_NotZ", "On-Shell: HH Not-Z-Pair Mass;Mass [GeV];Events");
    TLegend *leg_OnS = new TLegend(0.65, 0.5, 0.88, 0.88); leg_OnS->SetBorderSize(0);

    THStack *hs_OffS_Z = new THStack("hs_OffS_Z", "Off-Shell: HH Z-Pair Mass;Mass [GeV];Events");
    THStack *hs_OffS_NotZ = new THStack("hs_OffS_NotZ", "Off-Shell: HH Not-Z-Pair Mass;Mass [GeV];Events");
    TLegend *leg_OffS = new TLegend(0.65, 0.5, 0.88, 0.88); leg_OffS->SetBorderSize(0);

    THStack *hs_Comb_Z = new THStack("hs_Comb_Z", "Combined: HH Z-Pair Mass;Mass [GeV];Events");
    THStack *hs_Comb_NotZ = new THStack("hs_Comb_NotZ", "Combined: HH Not-Z-Pair Mass;Mass [GeV];Events");
    TLegend *leg_Comb = new TLegend(0.65, 0.5, 0.88, 0.88); leg_Comb->SetBorderSize(0);

    int globalIndex = 0;

    // --- B. Process Directories ---
    ProcessDirectory(
        "SelectionResults/OnShellCut/", "_OnS", 0, 145,
        hs_OnS_Z, hs_OnS_NotZ, hs_Comb_Z, hs_Comb_NotZ,
        leg_OnS, leg_Comb, globalIndex, bg_xsec
    );

    ProcessDirectory(
        "SelectionResults/CombineAll/", "_OffS", 150, 999,
        hs_OffS_Z, hs_OffS_NotZ, hs_Comb_Z, hs_Comb_NotZ,
        leg_OffS, leg_Comb, globalIndex, bg_xsec
    );

    // --- C. Print the requested Yield Table ---
    PrintYieldTable(bg_xsec);
    ExportYieldsForCombine(bg_xsec);

    // --- D. Write Output ---
    fOut->cd();

    TCanvas *c1 = new TCanvas("c_OnS_Z", "On-Shell Z Pair", 800, 600);
    hs_OnS_Z->Draw("nostack hist"); leg_OnS->Draw(); hm.WriteCanvas(c1);

    TCanvas *c2 = new TCanvas("c_OnS_NotZ", "On-Shell Not-Z Pair", 800, 600);
    hs_OnS_NotZ->Draw("nostack hist"); leg_OnS->Draw(); hm.WriteCanvas(c2);

    TCanvas *c3 = new TCanvas("c_OffS_Z", "Off-Shell Z Pair", 800, 600);
    hs_OffS_Z->Draw("nostack hist"); leg_OffS->Draw(); hm.WriteCanvas(c3);

    TCanvas *c4 = new TCanvas("c_OffS_NotZ", "Off-Shell Not-Z Pair", 800, 600);
    hs_OffS_NotZ->Draw("nostack hist"); leg_OffS->Draw(); hm.WriteCanvas(c4);

    TCanvas *c5 = new TCanvas("c_Comb_Z", "Combined Z Pair", 800, 600);
    hs_Comb_Z->Draw("nostack hist"); leg_Comb->Draw(); hm.WriteCanvas(c5);

    TCanvas *c6 = new TCanvas("c_Comb_NotZ", "Combined Not-Z Pair", 800, 600);
    hs_Comb_NotZ->Draw("nostack hist"); leg_Comb->Draw(); hm.WriteCanvas(c6);

    hm.WriteStack(hs_OnS_Z, "Stack_OnS_Z");
    hm.WriteStack(hs_OnS_NotZ, "Stack_OnS_NotZ");
    hm.WriteStack(hs_OffS_Z, "Stack_OffS_Z");
    hm.WriteStack(hs_OffS_NotZ, "Stack_OffS_NotZ");
    hm.WriteStack(hs_Comb_Z, "Stack_Comb_Z");
    hm.WriteStack(hs_Comb_NotZ, "Stack_Comb_NotZ");

    cout << "------------------------------------------------" << endl;
    cout << "Done. Output saved to: " << outputFile << endl;
    cout << "------------------------------------------------" << endl;

    fOut->Close();
}