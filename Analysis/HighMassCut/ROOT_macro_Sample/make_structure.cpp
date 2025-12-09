#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TRandom3.h>
#include <iostream>

void make_structure() {
    // 1. Create the Master File
    TFile *f = new TFile("filename.root", "RECREATE");

    // ==================================================
    // PART 1: The Histogram "Tree" (Actually a Folder)
    // ==================================================
    // We create a directory named "histgramtree"
    TDirectory *histDir = f->mkdir("histgramtree");
    
    // Go inside this directory
    histDir->cd();

    // Create histograms (they are automatically attached to the current directory)
    TH1F *h1 = new TH1F("hist1", "Pt Distribution", 100, 0, 100);
    TH1F *h2 = new TH1F("hist2", "Eta Distribution", 100, -3, 3);
    
    // Fill them with some dummy data
    h1->FillRandom("gaus", 1000);
    h2->FillRandom("gaus", 1000);

    // Go back to the top level (root of the file)
    f->cd();


    // ==================================================
    // PART 2: The Event Tree (Event-by-Event Data)
    // ==================================================
    TTree *tEvent = new TTree("eventtree", "Tree with Event Data");
    
    // Variables for this tree
    float event_pt;
    int   event_id;

    tEvent->Branch("pt", &event_pt, "pt/F");
    tEvent->Branch("id", &event_id, "id/I");

    // Simulate 10 events
    for (int i = 0; i < 10; ++i) {
        event_pt = i * 5.5;
        event_id = i;
        tEvent->Fill();
    }


    // ==================================================
    // PART 3: The Summary Tree (Global Stats)
    // ==================================================
    TTree *tSum = new TTree("summarizetree", "Tree with Summary Data");

    // Variables for this tree
    float total_luminosity;
    int   total_events_processed;
    
    tSum->Branch("lumi", &total_luminosity, "lumi/F");
    tSum->Branch("processed", &total_events_processed, "processed/I");

    // Fill it ONLY ONCE
    total_luminosity = 137.5; 
    total_events_processed = 1000000;
    tSum->Fill();


    // ==================================================
    // CLEANUP & SAVE
    // ==================================================
    // Write everything. 
    // Since histograms are in a directory, we must write the directory too.
    
    f->Write(); // Recursively writes all directories and trees
    f->Close();

    delete f;
}