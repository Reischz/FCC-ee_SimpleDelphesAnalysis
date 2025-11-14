// create a cut that not use Z-mass window roo macro
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <filesystem>

// Function cut for Higgs Mass >= 150 GeV that push Z bosons off-shell at centre-of-mass energy 240 GeV
void Z_off_shell_cut(TString inputfile="HLFV_125GeV.root", TString outputfile="HLFV_125GeV_Zoff.root") {
    // check input file
    if (!std::filesystem::exists(inputfile)) {
        std::cerr << "Input file does not exist: " << inputfile << std::endl;
        std::exit(1);
    }
    // check input file content is zombie
    TFile *f = TFile::Open(inputfile);
    if (f->IsZombie()) {
        std::cerr << "Input file is corrupted (zombie): " << inputfile << std::endl;
        std::exit(1);
    }
    TTree *t = (TTree*)f->Get("events");

    // Define variables
    Float_t H_mass, Z1_mass, Z2_mass;
    t->SetBranchAddress("H_mass", &H_mass);
    t->SetBranchAddress("Z1_mass", &Z1_mass);
    t->SetBranchAddress("Z2_mass", &Z2_mass);

    // Create output file and tree
    TFile *f_out = TFile::Open(outputfile, "RECREATE");
    TTree *t_out = t->CloneTree(0);

    // Loop over events and apply cut
    Long64_t nentries = t->GetEntries();
    for (Long64_t i = 0; i < nentries; i++) {
        t->GetEntry(i);
        
        // Apply off-shell Z cut
        if (Z1_mass < 70.0 || Z1_mass > 110.0 || Z2_mass < 70.0 || Z2_mass > 110.0) {
            t_out->Fill();
        }
    }

    // Write output tree to file
    f_out->cd();
    t_out->Write();
    f_out->Close();
    f->Close();
}