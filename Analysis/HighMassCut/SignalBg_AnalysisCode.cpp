#include <TFile.h>
#include <TString.h>
#include <TH1.h>
#include <TH2.h>
#include <iostream>
#include <map>

using namespace std;

// =========================================================================
// 1. HISTOGRAM MANAGER (Reusable Class)
// =========================================================================
struct HistManager {
    std::map<TString, TH1*> h1;
    std::map<TString, TH2*> h2;
    TDirectory* dir = nullptr;

    // Set the target directory (the ROOT file)
    void SetDirectory(TDirectory* d) { dir = d; }

    // Book 1D Histogram
    TH1F* Book1D(TString name, TString title, int nbins, double min, double max) {
        if (dir) dir->cd(); // Switch to file
        TH1F* h = new TH1F(name, title, nbins, min, max);
        h->SetDirectory(dir); // Ensure it saves to file
        h1[name] = h;         // Store in map for easy access
        return h;
    }

    // Book 2D Histogram
    TH2F* Book2D(TString name, TString title, int nxbins, double xmin, double xmax, int nybins, double ymin, double ymax) {
        if (dir) dir->cd();
        TH2F* h = new TH2F(name, title, nxbins, xmin, xmax, nybins, ymin, ymax);
        h->SetDirectory(dir);
        h2[name] = h;
        return h;
    }

    // Safe Fill Functions
    void Fill1D(TString name, double val) {
        if (h1.count(name)) h1[name]->Fill(val);
        else cout << "Warning: Hist " << name << " not found!" << endl;
    }

    void Fill2D(TString name, double valx, double valy) {
        if (h2.count(name)) h2[name]->Fill(valx, valy);
        else cout << "Warning: Hist " << name << " not found!" << endl;
    }
};

// =========================================================================
// 2. MAIN MACRO
// =========================================================================
void HistModel(TString outputfile = "Output_Histograms.root") {
    
    // --- 1. Create Output File ---
    TFile *fOut = TFile::Open(outputfile, "RECREATE");
    if (!fOut || fOut->IsZombie()) { cerr << "Error creating file" << endl; return; }

    // --- 2. Initialize Manager ---
    HistManager hm;
    hm.SetDirectory(fOut); // Tell manager to save hists here

    // --- 3. Book Histograms ---
    hm.Book1D("Lepton_PT",    "Lepton PT;p_{T} [GeV];Events",  100, 0, 200);
    hm.Book1D("Lepton_Eta",   "Lepton Eta;#eta;Events",        50, -3, 3);
    hm.Book2D("Mass_Heatmap", "Mass Check;Z Mass;H Mass",      50, 0, 150, 50, 0, 150);

    // --- 4. Fill Loop (Simulated Analysis) ---
    cout << "Filling histograms..." << endl;
    
    // In real code, this is: for(int i=0; i < tree->GetEntries(); i++)
    for (int i = 0; i < 1000; i++) {
        
        // Dummy values (Replace with: ev.Electron_PT[k])
        double pt  = 50 + (rand() % 100); 
        double eta = ((rand() % 60) - 30) / 10.0;
        double z_m = 91.0; 
        double h_m = 125.0;

        // The easy fill syntax:
        hm.Fill1D("Lepton_PT", pt);
        hm.Fill1D("Lepton_Eta", eta);
        hm.Fill2D("Mass_Heatmap", z_m, h_m);
    }

    // --- 5. Write & Close ---
    cout << "Writing to " << outputfile << "..." << endl;
    
    fOut->Write(); // Writes all booked histograms automatically
    fOut->Close();
    
    cout << "Done." << endl;
}