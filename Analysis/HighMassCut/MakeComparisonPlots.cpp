#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1.h>
#include <THStack.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TSystemDirectory.h>
#include <TList.h>
#include <TKey.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

// =========================================================================
// 1. REUSABLE HISTOGRAM MANAGER
// =========================================================================
struct HistManager {
    TDirectory* dir = nullptr;

    void SetDirectory(TDirectory* d) { dir = d; }

    // Helper to write a THStack to the file
    void WriteStack(THStack* stack, TString name) {
        if (dir) dir->cd();
        stack->Write(name);
    }

    // Helper to write a Canvas to the file
    void WriteCanvas(TCanvas* c) {
        if (dir) dir->cd();
        c->Write();
    }
};

// =========================================================================
// 2. HELPER FUNCTIONS
// =========================================================================

// Identify the Status branch with the highest index (e.g., "Status_06_...")
TString GetFinalCutBranchName(TTree* t) {
    TObjArray* branches = t->GetListOfBranches();
    TString bestBranch = "";
    int maxIndex = -1;

    for (int i = 0; i < branches->GetEntries(); i++) {
        TString bName = branches->At(i)->GetName();
        if (bName.BeginsWith("Status_")) {
            // Extract number between "Status_" and "_"
            TString numStr = bName(7, 2); // Get 2 chars after "Status_"
            if (numStr.IsDigit()) {
                int index = numStr.Atoi();
                if (index > maxIndex) {
                    maxIndex = index;
                    bestBranch = bName;
                }
            }
        }
    }
    return bestBranch;
}

// Check if file is relevant
bool IsAdditionalTree(TString filename) {
    return filename.EndsWith("AdditionalTree.root") && !filename.BeginsWith(".");
}

// Generate a color/style based on index to make them distinct
void StyleHistogram(TH1F* h, int index, TString name) {
    int colors[] = {kRed+1, kBlue+1, kGreen+2, kMagenta+1, kOrange+1, kCyan+1, kViolet, kBlack, kGray+2};
    int styles[] = {kSolid, kDashed, kDotted};
    
    int colorIdx = index % 9;
    int styleIdx = (index / 9) % 3;

    h->SetLineColor(colors[colorIdx]);
    h->SetLineWidth(2);
    h->SetLineStyle(styles[styleIdx]);
    
    // Fill with low opacity for layered look
    h->SetFillColorAlpha(colors[colorIdx], 0.1); 
    
    h->SetStats(0); // Turn off stat box for clean overlay
    h->SetTitle(name);
}

// =========================================================================
// 3. MAIN MACRO
// =========================================================================
void MakeComparisonPlots(
    TString inputDir = "SelectionResults/CombineAll/", 
    TString outputFile = "Comparison_Distributions.root"
) { 
    
    // --- 1. Setup Input/Output ---
    TSystemDirectory dir(inputDir, inputDir);
    TList *files = dir.GetListOfFiles();
    
    if (!files) {
        cerr << "Error: Directory '" << inputDir << "' not found." << endl;
        return;
    }

    TFile *fOut = TFile::Open(outputFile, "RECREATE");
    HistManager hm;
    hm.SetDirectory(fOut);

    // --- 2. Containers for Stacks ---
    THStack *hs_ZPair = new THStack("hs_ZPair", "HH Z-Pair Mass Distribution;Mass [GeV];Events");
    THStack *hs_NotZPair = new THStack("hs_NotZPair", "HH Not-Z-Pair Mass Distribution;Mass [GeV];Events");
    
    TLegend *leg = new TLegend(0.65, 0.5, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);

    // --- 3. Iterate over files ---
    files->Sort(); // Alphabetical order
    TIter next(files);
    TSystemFile *file;
    int fileIndex = 0;

    cout << "Reading files from: " << inputDir << endl;

    while ((file = (TSystemFile*)next())) {
        TString fname = file->GetName();
        if (!IsAdditionalTree(fname)) continue;

        // Clean Name for Legend (Remove _AdditionalTree.root)
        TString sampleName = fname;
        sampleName.ReplaceAll("_AdditionalTree.root", "");

        cout << "Processing: " << sampleName << " ... ";

        // Open File
        TFile *fIn = TFile::Open(inputDir + fname);
        if (!fIn || fIn->IsZombie()) { cout << " [Skipped: Corrupt]" << endl; continue; }

        TTree *t = (TTree*)fIn->Get("Selection Results");
        if (!t) { cout << " [Skipped: No Tree]" << endl; fIn->Close(); continue; }

        // Determine Final Cut Branch
        TString cutBranchName = GetFinalCutBranchName(t);
        if (cutBranchName == "") { cout << " [Skipped: No Status Branch]" << endl; fIn->Close(); continue; }
        
        // Setup Branches
        int passStatus = 0;
        float HH_ZPair_Mass = 0;
        float HH_NotZPair_Mass = 0;

        t->SetBranchAddress(cutBranchName, &passStatus);
        t->SetBranchAddress("HH_ZPair_Mass", &HH_ZPair_Mass);
        t->SetBranchAddress("HH_NotZPair_Mass", &HH_NotZPair_Mass);

        // Book Temporary Histograms
        // Note: Set directory to 0 initially so they don't get deleted when fIn closes
        TH1F *h_Z = new TH1F(sampleName + "_Z", sampleName, 50, 0, 250);
        TH1F *h_NotZ = new TH1F(sampleName + "_NotZ", sampleName, 50, 0, 250);
        h_Z->SetDirectory(0);
        h_NotZ->SetDirectory(0);

        // Event Loop
        Long64_t nentries = t->GetEntries();
        for (Long64_t i = 0; i < nentries; i++) {
            t->GetEntry(i);
            
            // >>> THE CUT <<<
            // 0=Fail, 1=Pass. We only fill if Status == 1
            if (passStatus == 1) {
                h_Z->Fill(HH_ZPair_Mass);
                h_NotZ->Fill(HH_NotZPair_Mass);
            }
        }

        // Style and Add to Stack
        StyleHistogram(h_Z, fileIndex, sampleName);
        StyleHistogram(h_NotZ, fileIndex, sampleName);

        hs_ZPair->Add(h_Z);
        hs_NotZPair->Add(h_NotZ);
        leg->AddEntry(h_Z, sampleName, "l");

        cout << "Done (Final Branch: " << cutBranchName << ")" << endl;
        
        fIn->Close();
        fileIndex++;
    }

    // --- 4. Write Output ---
    fOut->cd();

    // Option "nostack" draws them overlapping (layered), not stacked on top of each other
    // This allows comparing shapes directly
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

    cout << "------------------------------------------------" << endl;
    cout << "Results saved to: " << outputFile << endl;
    cout << "------------------------------------------------" << endl;

    fOut->Close();
}
