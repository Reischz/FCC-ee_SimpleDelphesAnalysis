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
        // Alternating styles for backgrounds to distinguish OnS vs OffS
        h->SetLineStyle(name.Contains("OnS") ? kDashed : kDotted); 
        h->SetFillColorAlpha(colors[colorIdx], 0.0); // No fill for BG
    }
    
    h->SetStats(0);
    h->SetTitle(name);
}

// =========================================================================
// 3. CORE PROCESSING FUNCTION
// =========================================================================
void ProcessDirectory(
    TString inputDir, 
    TString suffixLabel, 
    int minSignalMass, 
    int maxSignalMass,
    THStack* specificStackZ, 
    THStack* specificStackNotZ, 
    THStack* combinedStackZ, 
    THStack* combinedStackNotZ,
    TLegend* specificLeg,
    TLegend* combinedLeg,
    int& globalIndex // To keep colors distinct across directories
) {
    TSystemDirectory dir(inputDir, inputDir);
    TList *files = dir.GetListOfFiles();
    
    if (!files) {
        cerr << "Warning: Directory '" << inputDir << "' not found." << endl;
        return;
    }

    files->Sort();
    TIter next(files);
    TSystemFile *file;

    cout << ">>> Processing Directory: " << inputDir << " (Suffix: " << suffixLabel << ")" << endl;

    while ((file = (TSystemFile*)next())) {
        TString fname = file->GetName();
        if (!IsAdditionalTree(fname)) continue;

        TString baseName = fname;
        baseName.ReplaceAll("_AdditionalTree.root", "");

        // --- FILTERING LOGIC ---
        bool isSignal = baseName.Contains("HLFV");
        if (isSignal) {
            // Extract Mass: "HLFV_120GeV" -> 120
            TString massStr = baseName;
            massStr.ReplaceAll("HLFV_", "");
            massStr.ReplaceAll("GeV", "");
            int mass = massStr.Atoi();

            // Skip if outside requested range
            if (mass < minSignalMass || mass > maxSignalMass) continue;
        }
        
        // --- NAMING LOGIC ---
        // For Backgrounds: Append suffix (e.g., "ZHTaTa_OnS")
        // For Signals: Keep original name (e.g., "HLFV_110GeV") because they are unique
        TString uniqueName = baseName;
        if (!isSignal) uniqueName += suffixLabel;

        cout << "   -> Adding: " << uniqueName << endl;

        // --- ROOT PROCESSING ---
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

        // Book Histograms
        TH1F *h_Z = new TH1F(uniqueName + "_Z", uniqueName, 50, 0, 250);
        TH1F *h_NotZ = new TH1F(uniqueName + "_NotZ", uniqueName, 50, 0, 250);
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

        // Apply Style
        StyleHistogram(h_Z, globalIndex, uniqueName);
        StyleHistogram(h_NotZ, globalIndex, uniqueName);
        globalIndex++;

        // Add to SPECIFIC Stack (e.g., only OnShell plot)
        specificStackZ->Add(h_Z);
        specificStackNotZ->Add(h_NotZ);
        specificLeg->AddEntry(h_Z, uniqueName, "l");

        // Add to COMBINED Stack (The Big Plot)
        // We clone to allow independent modification if needed, or just add pointer
        // (Adding same pointer to two stacks is risky if stacks manage ownership, 
        // but simple THStack usually doesn't delete histograms automatically unless specified.
        // To be safe, we Clone for the second stack).
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

// =========================================================================
// 4. MAIN MACRO
// =========================================================================
void MakeComparisonPlots_Split(TString outputFile = "Comparison_Distributions_Full.root") { 
    
    TFile *fOut = TFile::Open(outputFile, "RECREATE");
    HistManager hm;
    hm.SetDirectory(fOut);

    // --- A. Define Stacks ---
    
    // 1. On-Shell Only (110-145)
    THStack *hs_OnS_Z = new THStack("hs_OnS_Z", "On-Shell: HH Z-Pair Mass;Mass [GeV];Events");
    THStack *hs_OnS_NotZ = new THStack("hs_OnS_NotZ", "On-Shell: HH Not-Z-Pair Mass;Mass [GeV];Events");
    TLegend *leg_OnS = new TLegend(0.65, 0.5, 0.88, 0.88); leg_OnS->SetBorderSize(0);

    // 2. Off-Shell Only (150-160)
    THStack *hs_OffS_Z = new THStack("hs_OffS_Z", "Off-Shell: HH Z-Pair Mass;Mass [GeV];Events");
    THStack *hs_OffS_NotZ = new THStack("hs_OffS_NotZ", "Off-Shell: HH Not-Z-Pair Mass;Mass [GeV];Events");
    TLegend *leg_OffS = new TLegend(0.65, 0.5, 0.88, 0.88); leg_OffS->SetBorderSize(0);

    // 3. Combined (All)
    THStack *hs_Comb_Z = new THStack("hs_Comb_Z", "Combined: HH Z-Pair Mass;Mass [GeV];Events");
    THStack *hs_Comb_NotZ = new THStack("hs_Comb_NotZ", "Combined: HH Not-Z-Pair Mass;Mass [GeV];Events");
    TLegend *leg_Comb = new TLegend(0.65, 0.5, 0.88, 0.88); leg_Comb->SetBorderSize(0);

    int globalIndex = 0; // Ensures colors don't reset between directories for the combined plot

    // --- B. Process Directory 1: On-Shell (110-145) ---
    // Dir: SelectionResults_OnS/
    // Suffix: _OnS
    // Range: 0 to 145 GeV
    ProcessDirectory(
        "SelectionResults/OnShellCut/", "_OnS", 0, 145,
        hs_OnS_Z, hs_OnS_NotZ, hs_Comb_Z, hs_Comb_NotZ,
        leg_OnS, leg_Comb, globalIndex
    );

    // --- C. Process Directory 2: CombineAll (150-160) ---
    // Dir: SelectionResults/CombineAll/
    // Suffix: _OffS
    // Range: 150 to 999 GeV
    ProcessDirectory(
        "SelectionResults/CombineAll/", "_OffS", 150, 999,
        hs_OffS_Z, hs_OffS_NotZ, hs_Comb_Z, hs_Comb_NotZ,
        leg_OffS, leg_Comb, globalIndex
    );

    // --- D. Write Output ---
    fOut->cd();

    // 1. Draw On-Shell
    TCanvas *c1 = new TCanvas("c_OnS_Z", "On-Shell Z Pair", 800, 600);
    hs_OnS_Z->Draw("nostack hist"); leg_OnS->Draw(); hm.WriteCanvas(c1);

    TCanvas *c2 = new TCanvas("c_OnS_NotZ", "On-Shell Not-Z Pair", 800, 600);
    hs_OnS_NotZ->Draw("nostack hist"); leg_OnS->Draw(); hm.WriteCanvas(c2);

    // 2. Draw Off-Shell
    TCanvas *c3 = new TCanvas("c_OffS_Z", "Off-Shell Z Pair", 800, 600);
    hs_OffS_Z->Draw("nostack hist"); leg_OffS->Draw(); hm.WriteCanvas(c3);

    TCanvas *c4 = new TCanvas("c_OffS_NotZ", "Off-Shell Not-Z Pair", 800, 600);
    hs_OffS_NotZ->Draw("nostack hist"); leg_OffS->Draw(); hm.WriteCanvas(c4);

    // 3. Draw Combined
    TCanvas *c5 = new TCanvas("c_Comb_Z", "Combined Z Pair", 800, 600);
    hs_Comb_Z->Draw("nostack hist"); leg_Comb->Draw(); hm.WriteCanvas(c5);

    TCanvas *c6 = new TCanvas("c_Comb_NotZ", "Combined Not-Z Pair", 800, 600);
    hs_Comb_NotZ->Draw("nostack hist"); leg_Comb->Draw(); hm.WriteCanvas(c6);

    // Write Stacks
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