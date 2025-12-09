#include <TFile.h>
#include <TH1F.h>
#include <TRandom3.h>
#include <iostream>

void save_hist() {
    // 1. Create the Histogram
    TH1F *h1 = new TH1F("h_energy", "Energy Distribution;Energy [GeV];Counts", 100, 0, 100);

    // 2. Fill it with some random dummy data
    TRandom3 randGen(42);
    for (int i = 0; i < 10000; ++i) {
        h1->Fill(randGen.Gaus(50, 10)); // Gaussian centered at 50, width 10
    }

    // 3. Open a ROOT file for writing
    // "RECREATE" means: create a new file, overwrite if it exists.
    TFile *f_out = new TFile("my_results.root", "RECREATE");

    // 4. Check if file opened successfully (Good practice)
    if (!f_out || f_out->IsZombie()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    // 5. Write the histogram to the file
    // Note: You can call h1->Write() explicitly, or f_out->Write() to save everything in memory.
    // Explicit is usually safer.
    h1->Write();

    // 6. Close the file
    // This effectively saves the data and frees the file pointer.
    f_out->Close();

    // Clean up memory (Optional in macros, but good for compiled code)
    delete f_out; 
    // Do NOT delete h1 before closing the file if you rely on ROOT's directory auto-management!
    // But since we called h1->Write(), h1 is safe to delete here if needed.
    delete h1;

    std::cout << "Histogram saved to my_results.root" << std::endl;
}