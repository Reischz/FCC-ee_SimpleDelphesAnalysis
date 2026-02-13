#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1.h>
#include <THStack.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TSystemDirectory.h>
#include <TList.h>
#include <TStyle.h> // Added for TStyle
#include <iostream>

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

// *** NEW STYLING FUNCTION ***
void StyleHistogram(TH1F* h, int signalIndex, int bkgIndex, TString name) {
    
    // Check if it matches the pattern HLFV_...GeV
    // We check for "HLFV_" and "GeV"
    bool isSignal = name.Contains("HLFV_") && name.Contains("GeV");

    if (isSignal) {
        // --- SIGNAL STYLE (Solid, Warm Colors) ---
        h->SetLineStyle(kSolid);
        h->SetLineWidth(3); // Thicker for emphasis
        
        // Gradient: Orange -> Red -> Pink -> Magenta
        // This keeps them in the "Warm" family (Same shade vibe)
        int warmColors[] = {kOrange+1, kOrange+7, kRed-4, kRed, kRed+2, kPink+9, kMagenta, kViolet, kViolet+2};
        int nColors = sizeof(warmColors)/sizeof(int);
        
        int color = warmColors[signalIndex % nColors];
        h->SetLineColor(color);
        
        // Optional: Very light fill to make them pop, but keep transparent
        h->SetFillColorAlpha(color, 0.05); 
    
    } else {
        // --- BACKGROUND STYLE (Dashed, Pale/Cool Colors) ---
        h->SetLineStyle(7); // 7 is a nice medium dash (kDashed is often too dense)
        h->SetLineWidth(2);
        
        // Gradient: Cyan -> Blue -> Grey (Cool/Pale family)
        // Using -X modifiers makes them lighter/paler
        int coolColors[] = {kCyan-2, kAzure-4, kBlue-7, kBlue-9, kTeal-6, kGray+1};
        int nColors = sizeof(coolColors)/sizeof(int);
        
        int color = coolColors[bkgIndex % nColors];
        h->SetLineColor(color);
        
        // No fill or extremely faint fill for background
        h->SetFillColorAlpha(color, 0.0);
    }
    
    h->SetStats(0);
    h->SetTitle(name);
}

// =========================================================================
// 3. MAIN MACRO
// =========================================================================
void MakeComparisonPlots(
    TString inputDir = "SelectionResults/CombineAll/", 
    TString outputFile = "Comparison_Distributions.root"
) { 
    // Set global style for better looking plots
    gStyle->SetOptStat(0);
    
    TSystemDirectory dir(inputDir, inputDir);
    TList *files = dir.GetListOfFiles();
    if (!files) { cerr << "Error: Directory not found." << endl; return; }

    TFile *fOut = TFile::Open(outputFile, "RECREATE");
    HistManager hm;
    hm.SetDirectory(fOut);

    THStack *hs_ZPair = new THStack("hs_ZPair", "HH Z-Pair Mass;Mass [GeV];Events");
    THStack *hs_NotZPair = new THStack("hs_NotZPair", "HH Not-Z-Pair Mass;Mass [GeV];Events");
    
    TLegend *leg = new TLegend(0.60, 0.55, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.025); // Slightly smaller text to fit many entries

    files->Sort();
    TIter next(files);
    TSystemFile *file;
    
    // Separate counters for gradients
    int signalCount = 0;
    int bkgCount = 0;

    cout << "Reading files from: " << inputDir << endl;

    while ((file = (TSystemFile*)next())) {
        TString fname = file->GetName();
        if (!IsAdditionalTree(fname)) continue;

        TString sampleName = fname;
        sampleName.ReplaceAll("_AdditionalTree.root", "");

        cout << "Processing: " << sampleName << " ... ";

        TFile *fIn = TFile::Open(inputDir + fname);
        if (!fIn || fIn->IsZombie()) { cout << " [Skipped]" << endl; continue; }
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

        TH1F *h_Z = new TH1F(sampleName + "_Z", sampleName, 50, 0, 250);
        TH1F *h_NotZ = new TH1F(sampleName + "_NotZ", sampleName, 50, 0, 250);
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

        // Apply Style based on pattern
        // We increment counters AFTER using them to cycle colors
        bool isSignal = sampleName.Contains("HLFV_") && sampleName.Contains("GeV");
        
        StyleHistogram(h_Z, signalCount, bkgCount, sampleName);
        StyleHistogram(h_NotZ, signalCount, bkgCount, sampleName);
        
        if (isSignal) signalCount++;
        else bkgCount++;

        hs_ZPair->Add(h_Z);
        hs_NotZPair->Add(h_NotZ);
        leg->AddEntry(h_Z, sampleName, "l"); // "l" means Line only (good for dashes)

        cout << "Done" << endl;
        fIn->Close();
    }

    fOut->cd();
    
    // Draw Option "nostack" is CRITICAL for comparison
    TCanvas *c1 = new TCanvas("c_ZPair", "Z Pair Mass", 800, 600);
    hs_ZPair->Draw("nostack hist"); 
    leg->Draw();
    hm.WriteCanvas(c1);
    hm.WriteStack(hs_ZPair, "Stack_ZPair");

    TCanvas *c2 = new TCanvas("c_NotZPair", "Not Z Pair Mass", 800, 600);
    hs_NotZPair->Draw("nostack hist");
    leg->Draw();
    hm.WriteCanvas(c2);
    hm.WriteStack(hs_NotZPair, "Stack_NotZPair");

    fOut->Close();
    cout << "Done. Output saved to " << outputFile << endl;
}