

void printElectronPTs() {
    // Load the ROOT file
    TFile *file = TFile::Open("output01.root");
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    // Get the TTree (change the name if needed)
    TTree *t = (TTree*)file->Get("Delphes");
    if (!t) {
        std::cerr << "TTree 'Delphes' not found in file!" << std::endl;
        return;
    }

    // Declare pointer to the vector that will hold Electron.PT values
    std::vector<float> *Electron_PT = nullptr;

    // Set the branch address
    t->SetBranchAddress("Electron.PT", &Electron_PT);

    // Loop over all entries (events)
    Long64_t nentries = t->GetEntries();
    for (Long64_t i = 0; i < nentries; ++i) {
        t->GetEntry(i);  // Load i-th event
        std::cout << "Event " << i << ": ";
        if (Electron_PT) {
            for (size_t j = 0; j < Electron_PT->size(); ++j) {
                std::cout << (*Electron_PT)[j] << " ";
            }
        }
        std::cout << std::endl;
    }

    // Cleanup
    file->Close();
}
