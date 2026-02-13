#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1F.h>
#include <THStack.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

// =========================================================================
// HELPER: Get Final Cut Branch Name dynamically
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

// =========================================================================
// HELPER: Get Yield from a file using TTree::GetEntries
// =========================================================================
double GetYield(TString filepath, double targetMass, double massWindow, double weight) {
    TFile* f = TFile::Open(filepath);
    if (!f || f->IsZombie()) {
        cerr << "Warning: Could not open " << filepath << endl;
        return 0.0;
    }

    TTree* t = (TTree*)f->Get("Selection Results");
    if (!t) { f->Close(); return 0.0; }

    TString statusBranch = GetFinalCutBranchName(t);
    if (statusBranch == "") { f->Close(); return 0.0; }

    // Build the selection string (equivalent to the Python mask)
    // e.g., "Status_08_NotZ_MassThreshold == 1 && abs(HH_NotZPair_Mass - 110) < 30"
    TString cutString = TString::Format("%s == 1 && abs(HH_NotZPair_Mass - %f) < %f", 
                                        statusBranch.Data(), targetMass, massWindow);

    // GetEntries counts how many events pass the cut string
    double count = t->GetEntries(cutString);
    
    f->Close();
    return count * weight;
}

// =========================================================================
// MAIN MACRO
// =========================================================================
void PlotYields(TString inputDir = "SelectionResults_OnS/") { // Change dir as needed
    
    // 1. Define Physics Parameters
    vector<int> signalMasses = {110, 115, 120, 125, 130, 135, 140, 145};
    double massWindow = 30.0;
    double signalWeight = 1e-6; // 1 ab^-1 for 1 million generated events

    map<TString, double> bg_xsec = {
        {"ZHTaTa", 2.19e-5},
        {"ZHWW",   7.84e-5},
        {"ZZTaTa", 1.52e-4},
        {"ZWW4l",  2.691e-6}
    };

    // Colors mapping (mimicking Viridis + Red for signal)
    map<TString, int> bg_colors = {
        {"ZWW4l",  kTeal+3},   // Dark Green/Teal
        {"ZHTaTa", kAzure-4},  // Blue
        {"ZHWW",   kSpring-5}, // Light Green
        {"ZZTaTa", kOrange-3}  // Yellow/Orange
    };

    int numBins = signalMasses.size();

    // 2. Initialize Histograms (Bar Charts)
    TH1F* h_signal = new TH1F("h_signal", "Signal", numBins, 0, numBins);
    h_signal->SetFillColor(kRed);
    h_signal->SetLineColor(kBlack);

    map<TString, TH1F*> h_bgs;
    for (auto const& [bgName, xsec] : bg_xsec) {
        h_bgs[bgName] = new TH1F("h_" + bgName, bgName, numBins, 0, numBins);
        h_bgs[bgName]->SetFillColor(bg_colors[bgName]);
        h_bgs[bgName]->SetLineColor(kBlack);
    }

    // 3. Process Data
    cout << "Calculating yields at 1 ab^{-1}..." << endl;
    for (int i = 0; i < numBins; i++) {
        int mass = signalMasses[i];
        TString binLabel = TString::Format("%d GeV", mass);

        // Set X-axis labels
        h_signal->GetXaxis()->SetBinLabel(i+1, binLabel);
        for (auto& [bgName, h_bg] : h_bgs) {
            h_bg->GetXaxis()->SetBinLabel(i+1, binLabel);
        }

        // Calculate Signal Yield
        TString sigFile = inputDir + TString::Format("HLFV_%dGeV_AdditionalTree.root", mass);
        double sigYield = GetYield(sigFile, mass, massWindow, signalWeight);
        h_signal->SetBinContent(i+1, sigYield);
        cout << "Mass " << mass << " Signal Yield: " << sigYield << endl;

        // Calculate Background Yields for this mass window
        for (auto const& [bgName, xsec] : bg_xsec) {
            TString bgFile = inputDir + bgName + "_AdditionalTree.root";
            double bgYield = GetYield(bgFile, mass, massWindow, xsec);
            h_bgs[bgName]->SetBinContent(i+1, bgYield);
        }
    }

    // 4. Plotting (Stacked Bar Chart)
    gStyle->SetOptStat(0);
    TCanvas *c1 = new TCanvas("c1", "Yields Chart", 1000, 600);
    c1->SetLogy(); // Set Y-axis to Log Scale
    c1->SetBottomMargin(0.15); // Make room for X labels

    THStack *hs = new THStack("hs", "Yields at 1 ab^{-1} after Final Mass Window Cut;Higgs Mass Hypothesis;Expected Events Yields");

    // Add Backgrounds to stack first (so they are at the bottom)
    for (auto const& [bgName, xsec] : bg_xsec) {
        hs->Add(h_bgs[bgName]);
    }
    // Add Signal on top
    hs->Add(h_signal);

    // "HIST" draws it as a bar chart (bins filled). 
    hs->Draw("HIST");

    // Adjust Y-axis minimum to make log scale look good
    hs->SetMinimum(1e-7); 
    
    // 5. Legend
    TLegend *leg = new TLegend(0.85, 0.6, 0.98, 0.9);
    leg->SetHeader("Process", "C");
    leg->AddEntry(h_signal, "Signal", "f");
    for (auto it = bg_xsec.rbegin(); it != bg_xsec.rend(); ++it) {
        leg->AddEntry(h_bgs[it->first], it->first, "f");
    }
    leg->Draw();

    c1->Update();
    
    // Save to file
    c1->SaveAs("Yields_StackedBarChart.png");
    c1->SaveAs("Yields_StackedBarChart.root");
    
    cout << "Plot saved to Yields_StackedBarChart.png" << endl;
}