#include <TParameter.h> // Required header

void save_summary_data() {
    // 1. Create File
    TFile *f = new TFile("summary_data.root", "RECREATE");

    // --- Assume you calculated this after your loop ---
    float calculated_median = 91.1876; 

    // 2. Create a TParameter Object
    // Arguments: "Name of the value", value
    TParameter<float> *medianParam = new TParameter<float>("MedianMass", calculated_median);

    // 3. Write it directly to the file
    medianParam->Write();

    // 4. Close
    f->Close();
    delete medianParam;
    delete f;
}