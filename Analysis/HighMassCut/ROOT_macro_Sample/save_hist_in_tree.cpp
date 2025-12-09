#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TRandom3.h>
#include <iostream>

void save_hist_in_tree() {
    TFile *f = new TFile("plots_in_tree.root", "RECREATE");
    TTree *t = new TTree("RunTree", "Tree containing a histogram per entry");

    // 1. Create a pointer to the object class
    // Initialize it to nullptr or a default object
    TH1F *h_current = new TH1F("h_temp", "Temp Title", 100, 0, 100);

    // 2. Create the Branch
    // Syntax: Branch("BranchName", "ClassName", &PointerAddress)
    // We use a buffer size of 32000 and split level 0 (keeps the object whole)
    t->Branch("CalibrationPlot", "TH1F", &h_current, 32000, 0);

    TRandom3 randGen(0);

    // 3. Loop (Simulating 10 "Runs")
    for (int i = 0; i < 10; ++i) {
        std::cout << "Processing Run " << i << "..." << std::endl;

        // A. Reset the histogram for this new entry
        h_current->Reset();
        
        // B. Update the Name/Title (Optional, but helpful)
        h_current->SetName(Form("h_run_%d", i));
        h_current->SetTitle(Form("Energy Distribution for Run %d;Energy;Counts", i));

        // C. Fill the histogram with unique data for this "run"
        // (Run 0 has mean 20, Run 1 has mean 30, etc.)
        float mean = 20.0 + (i * 10.0); 
        for (int k = 0; k < 1000; ++k) {
            h_current->Fill(randGen.Gaus(mean, 5.0));
        }

        // D. Fill the Tree
        // ROOT serializes the current state of 'h_current' into the tree
        t->Fill();
    }

    // 4. Save and Close
    t->Write();
    f->Close();
    
    // Cleanup
    delete f;
    // h_current is deleted when file closes if attached, 
    // or manually here if detached. Safer to let file handle it or delete last.
}