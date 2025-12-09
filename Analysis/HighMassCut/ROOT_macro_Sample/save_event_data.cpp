void save_event_data() {
    // 1. Create File and Tree
    TFile *f = new TFile("event_data.root", "RECREATE");
    TTree *t = new TTree("AnalysisTree", "Tree containing event-by-event data");

    // 2. Define the variable to hold the data
    float current_met;

    // 3. Create the Branch
    // Syntax: Branch("BranchName", &AddressOfVariable, "LeafList/Type")
    // /F = float, /D = double, /I = int
    t->Branch("MissingET", &current_met, "MissingET/F");

    // 4. Loop (Simulate your analysis loop)
    for (int i = 0; i < 100; ++i) {
        // Calculate your value for this event
        current_met = i * 2.5 + 5.0; 

        // Save this specific snapshot
        t->Fill(); 
    }

    // 5. Write and Close
    t->Write();
    f->Close();
}