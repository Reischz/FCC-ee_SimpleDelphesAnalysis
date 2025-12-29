#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TApplication.h> // Needed for clean ROOT termination
#include <iostream>
#include <cstdlib>        // Needed for std::exit
#include <filesystem>
#include <vector>
#include "TLorentzVector.h" // This is the one currently missing
#include "TH1.h"
#include "TH2.h"
#include <lib/selectionlist.h>
// #include <lib/parameter.h>
// Add any other ROOT headers you use (TH1F.h, TChain.h, etc.)

using namespace std;
// set beginning time
auto start_time = std::chrono::high_resolution_clock::now();

// First Cut: All leptons PT > 10 GeV
// Second Cut: 4 leptons in the event
// Third Cut: Each lepton is odd numbered (1,3,5...) in the collection
// Fourth Cut: No Charge violation in the event
// Fifth cut: Z windows +- 10 GeV around Z mass (91.1876 GeV)
// ==========================================
// Workflow Modular Design
// ==========================================
using AnalysisStep = std::pair<AnalysisModule*, bool>;
std::vector<AnalysisStep> ConfigurePipeline() {
    // Return the list directly!
    return {
        { new Lepton_PT()           ,    true  },
        { new FinalState_4Leptons() ,    true  }, 
        { new Lepton_Odd()          ,    true  },
        { new Charge_Violation()    ,    true  },
        { new Z_Window()            ,    true  },
        { new NotZ_dR()             ,    true  }, // Placeholder for future modules
        { new NotZ_MET_dPhi()       ,    true  }, // Disabled module example
        { new NotZ_MassThreshold()  ,    true  }
    };
}
// ==========================================
// Main Execution (ROOT Macro)
// ==========================================
void Z_off_shell_cut(TString inputfile="HLFV_125GeV.root", TString outputfile="HLFV_125GeV_Zoff.root", 
    TString TreeOutput="HLFV_125GeV_AdditionalTree.root") {
    
    // ===============================
    // Setting up input (Root) file
    // ===============================
    // check input file existence
    if (!std::filesystem::exists(inputfile.Data())) {
        cerr << "Input file does not exist: " << inputfile << endl;
        // It is safer to use simple return in interactive ROOT, 
        // but exit(1) works for batch jobs.
        exit(1); 
    }

    // check input file content is zombie
    TFile *f = TFile::Open(inputfile);
    if (!f || f->IsZombie()) {
        cerr << "Input file is corrupted (zombie): " << inputfile << endl;
        exit(1);
    }
    
    cout << "Applying Z off-shell cut on file: " << inputfile << endl;
    
    // Get the tree
    TTree *t = (TTree*)f->Get("Delphes");
    
    // Safety check: ensure tree exists
    if (!t) {
        cerr << "Error: Tree 'Delphes' not found in file!" << endl;
        f->Close();
        exit(1);
    }
    
    // --- SPEED OPTIMIZATION START ---
    // 1. Disable ALL branches first
    t->SetBranchStatus("*", 0);
    
    // 2. Enable ONLY the branches you actually use
    t->SetBranchStatus("Event_size", 1);
    t->SetBranchStatus("Electron*", 1); // Enables all branches starting with Electron
    t->SetBranchStatus("Muon*", 1);     // Enables all branches starting with Muon
    // t->SetBranchStatus("MissingET.MET", 1);
    t->SetBranchStatus("MissingET.Phi", 1);
    // --- SPEED OPTIMIZATION END ---

    // Define variables
    EventContext currentEvent;
    defaultParameters params;
    auto pipeline=ConfigurePipeline();
    
    // Make sure these branches actually exist before setting address
    if (t->GetBranch("Event_size"))  t->SetBranchAddress("Event_size", &currentEvent.Event_size);
    if (t->GetBranch("Electron_size")) t->SetBranchAddress("Electron_size", &currentEvent.Electron_size);
    if (t->GetBranch("Electron.PT")) t->SetBranchAddress("Electron.PT", &currentEvent.Electron_PT);
    if (t->GetBranch("Electron.Eta")) t->SetBranchAddress("Electron.Eta", &currentEvent.Electron_Eta);
    if (t->GetBranch("Electron.Phi")) t->SetBranchAddress("Electron.Phi", &currentEvent.Electron_Phi);
    if (t->GetBranch("Electron.Charge")) t->SetBranchAddress("Electron.Charge", &currentEvent.Electron_Charge);
    if (t->GetBranch("Muon_size")) t->SetBranchAddress("Muon_size", &currentEvent.Muon_size);
    if (t->GetBranch("Muon.PT")) t->SetBranchAddress("Muon.PT", &currentEvent.Muon_PT);
    if (t->GetBranch("Muon.Eta")) t->SetBranchAddress("Muon.Eta", &currentEvent.Muon_Eta);
    if (t->GetBranch("Muon.Phi")) t->SetBranchAddress("Muon.Phi", &currentEvent.Muon_Phi);
    if (t->GetBranch("Muon.Charge")) t->SetBranchAddress("Muon.Charge", &currentEvent.Muon_Charge);
    if (t->GetBranch("MissingET.Phi")) t->SetBranchAddress("MissingET.Phi", &currentEvent.MET_Phi);

    //================================
    // Setting up new tree for contains cut results
    //================================
    TFile *tfile_out = TFile::Open(TreeOutput, "RECREATE");
    tfile_out->cd();
    TTree *t_out = new TTree("Selection Results", "Tree with Z off-shell cut results");
    int dummy = 0;
     // Create branches for each cut in the pipeline
    for (auto& step : pipeline) {
        if (step.second) { // If module is active
            TString branchName = TString::Format("Status_%02d_%s", dummy, step.first->getName().c_str()); 
            // e.g., Cut_00_Lepton_PT, Cut_1_FinalState_4Leptons, etc.
            t_out->Branch(branchName, &currentEvent.CutStatus[dummy], (branchName + "/I"));
            dummy++;
        }
    }
    t_out->Branch("NearestZ_Mass", &currentEvent.NearestZ_Mass, "NearestZ_Mass/F");
    t_out->Branch("OtherPair_Mass", &currentEvent.OtherPair_Mass, "OtherPair_Mass/F");
    t_out->Branch("Z_PairIndexSum", &currentEvent.Z_PairIndexSum, "Z_PairIndexSum/I");
    t_out->Branch("NotZ_dR", &currentEvent.NotZ_dR, "NotZ_dR/F");
    t_out->Branch("NotZ_dPhi", &currentEvent.NotZ_dPhi, "NotZ_dPhi/F");
    t_out->Branch("NotZ_EleMET_dPhi", &currentEvent.NotZ_EleMET_dPhi, "NotZ_EleMET_dPhi/F");
    t_out->Branch("NotZ_MuMET_dPhi", &currentEvent.NotZ_MuMET_dPhi, "NotZ_MuMET_dPhi/F");
    // ==============================
    
    //=============================
    // Histogram Definition
    //=============================
    TFile *f_out = TFile::Open(outputfile, "RECREATE");
    f_out->cd();
    TDirectory *histDir = f_out->mkdir("histgramtree");
    // int dummy=0;
    vector<float> ZBIN={params.Z_MASS-params.Z_WINDOW, params.Z_MASS+params.Z_WINDOW};
    vector<TString> histNames={"Lepton.PT","Lepton.Eta","Lepton.Phi"};
    vector<TString> massHistNames={"NearestZ_Mass","OtherPair_Mass","NotZ_dR","NotZ_dPhi"};
    vector<TString> histXLabels={"GeV"," "," ","GeV","GeV"," "," "};
    vector<int> histNBins={100,100,100,50,160,50,50};
    vector<float> histXMin={0, -3, -5, ZBIN[0], 0, 0, 0};
    vector<float> histXMax={200, 3, 5, ZBIN[1], 160, 6, 3.5};
     // Create histograms for each cut in the pipeline
    dummy=0;
    size_t histNames_size=histNames.size();
    for (auto& step : pipeline) {
            if (step.second) { // If module is active
                if (currentEvent.readyformasshist){
                    for (size_t histidx=0; histidx<massHistNames.size(); histidx++){
                        TString histName = TString::Format("%02d_%s", dummy, massHistNames[histidx].Data());
                        TH1F *hist = new TH1F(histName, histName + ";" + histXLabels[histidx+histNames_size] + ";Events",
                            histNBins[histidx+histNames_size], histXMin[histidx+histNames_size], histXMax[histidx+histNames_size]);
                        hist->SetDirectory(histDir); // Associate histogram with directory
                    }
                    TH2F *vsmass = new TH2F(TString::Format("%02d_MassPairHeatmap", dummy), "Mass Map;Pair 1 Mass (GeV);Pair 2 Mass (GeV)", 50, ZBIN[0], ZBIN[1], 160, 0, 160);
                    vsmass->SetDirectory(histDir); // Associate histogram with directory
                }
                for (size_t histidx=0; histidx<histNames.size(); histidx++){
                    TString histName = TString::Format("%02d_%s", dummy, histNames[histidx].Data());
                    TH1F *hist = new TH1F(histName, histName + ";" + histXLabels[histidx] + ";Events",
                        histNBins[histidx], histXMin[histidx], histXMax[histidx]);
                    hist->SetDirectory(histDir); // Associate histogram with directory
                }
            }
            dummy++;
        }

    // ==============================
    // Loop over events and apply cut
    Long64_t nentries = t->GetEntries();
    cout << "Processing " << nentries << " entries..." << endl;
    // variable for summary selection
    vector<int> passcut;

    int selection_counts[5] = {0, 0, 0, 0, 0};
    for (Long64_t i = 0; i < nentries; i++) {
        currentEvent.reset();
        t->GetEntry(i);
        // Check branchMap for the first event
        dummy=0;
        for (auto& step : pipeline) {
            if (step.second) { // If module is active
                step.first->process(currentEvent, params);
                if (currentEvent.PassThisCut == false) {
                    break; // Stop processing further modules
                }
                currentEvent.CutStatus[currentEvent.CurrentCut] = currentEvent.PassThisCut ? 1 : 0;
                for (size_t histidx=0; histidx<histNames.size(); histidx++){
                    TString histName = TString::Format("%02d_%s", dummy, histNames[histidx].Data());
                    TH1F *hist = (TH1F*)histDir->Get(histName);
                    if (hist) {
                        if (histNames[histidx]=="Lepton.PT"){
                            for (int e=0; e<currentEvent.Electron_size; e++){
                                hist->Fill(currentEvent.Electron_PT[e]);
                            }
                            for (int m=0; m<currentEvent.Muon_size; m++){
                                hist->Fill(currentEvent.Muon_PT[m]);
                            }
                        } else if (histNames[histidx]=="Lepton.Eta"){
                            for (int e=0; e<currentEvent.Electron_size; e++){
                                hist->Fill(currentEvent.Electron_Eta[e]);
                            }
                            for (int m=0; m<currentEvent.Muon_size; m++){
                                hist->Fill(currentEvent.Muon_Eta[m]);
                            }
                        } else if (histNames[histidx]=="Lepton.Phi"){
                            for (int e=0; e<currentEvent.Electron_size; e++){
                                hist->Fill(currentEvent.Electron_Phi[e]);
                            }
                            for (int m=0; m<currentEvent.Muon_size; m++){
                                hist->Fill(currentEvent.Muon_Phi[m]);
                            }
                        } else if (histNames[histidx]=="NearestZ_Mass"){
                            hist->Fill(currentEvent.NearestZ_Mass);
                        } else if (histNames[histidx]=="OtherPair_Mass"){
                            hist->Fill(currentEvent.OtherPair_Mass);
                        }
                    }
                }
                if (currentEvent.readyformasshist){
                    // Fill mass histograms
                    for (size_t histidx=0; histidx<massHistNames.size(); histidx++){
                        TString histName = TString::Format("%02d_%s", dummy, massHistNames[histidx].Data());
                        TH1F *hist = (TH1F*)histDir->Get(histName);
                        if (hist) {
                            if (massHistNames[histidx]=="NearestZ_Mass"){
                                hist->Fill(currentEvent.NearestZ_Mass);
                            } else if (massHistNames[histidx]=="OtherPair_Mass"){
                                hist->Fill(currentEvent.OtherPair_Mass);
                            } else if (massHistNames[histidx]=="NotZ_dR"){
                                hist->Fill(currentEvent.NotZ_dR);
                            } else if (massHistNames[histidx]=="NotZ_dPhi"){
                                hist->Fill(currentEvent.NotZ_dPhi);
                            }
                        }
                    }
                    TH2F *vsmass = (TH2F*)histDir->Get(TString::Format("%02d_MassPairHeatmap", dummy));
                    if (vsmass) {
                        vsmass->Fill(currentEvent.NearestZ_Mass, currentEvent.OtherPair_Mass);
                    }
                }
                currentEvent.CurrentCut++;
            }
            dummy++;
        }
        // Fill the output tree
        t_out->Fill();
        // Find index of 1 in CutStatus
        for (int i=0; i < 10; i++){
            if (currentEvent.CutStatus[i]==1){
                selection_counts[i]=selection_counts[i]+1;
            }
        }
        // report processed status every 100k events
        if (i % 100000 == 0) {
            cout << " Processed " << i << " events..." << endl;
        }
    }
    // ==============================
    // Cleanup
    // ==============================
    for (auto& step : pipeline) {
        delete step.first;
    }
    // Close input file
    f->Close();
    // ==============================
    // Summary of selection
    // ==============================

    cout << "Summary of selection:" << endl;
    cout << " Total events processed: " << nentries << endl;
    cout << " Events passing each cut stage:" << endl;
    cout << "  After Lepton PT cut: " << selection_counts[0] << endl;
    cout << "  After Final State 4 Leptons cut: " << selection_counts[1] << endl;
    cout << "  After Lepton Odd cut: " << selection_counts[2] << endl;
    cout << "  After Charge Violation cut: " << selection_counts[3] << endl;
    cout << "  After Z Window cut: " << selection_counts[4] << endl;

    // test loop overhead time without calculation
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    cout << "Loop overhead time: " << elapsed.count() << " seconds" << endl;
    //==============================
    // Write Tree Output
    //==============================
    tfile_out->cd();
    t_out->Write();
    tfile_out->Close();

    //==============================
    // Write output histogram to file
    //==============================
    f_out->cd();
    histDir->Write(); // Write all histograms in the directory
    // Clean up
    cout << "Done. Output written to: " << outputfile << endl;
    f_out->Close();
    // t_out->Close();
    //==============================
    gApplication->Terminate(); // Clean ROOT termination
}