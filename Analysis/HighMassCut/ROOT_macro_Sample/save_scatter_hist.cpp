#include <TFile.h>
#include <TH2F.h>
#include <TRandom3.h>

void save_scatter_hist() {
    // 1. Create the 2D Histogram
    // Arguments: Name, Title, X-bins, X-min, X-max, Y-bins, Y-min, Y-max
    TH2F *h2 = new TH2F("h2_density", "Event Density;Mass [GeV];PT [GeV]", 
                        50, 0, 100,  // X Axis settings
                        50, 0, 200); // Y Axis settings

    // 2. Fill it inside your loop
    TRandom3 randGen(0);
    for (int i = 0; i < 10000; ++i) {
        double mass = randGen.Gaus(50, 10);
        double pt   = randGen.Landau(40, 5);
        
        // Just pass both X and Y to Fill()
        h2->Fill(mass, pt);
    }

    // 3. Save to File
    TFile *f_out = new TFile("scatter_hist.root", "RECREATE");
    
    h2->Write(); // It already has a name ("h2_density")

    f_out->Close();
    delete f_out;
    delete h2;
}