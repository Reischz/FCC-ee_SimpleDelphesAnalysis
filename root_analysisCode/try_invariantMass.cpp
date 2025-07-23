#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"

void plot_allFinalProcessInfo()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	TH1F *invmass = new TH1F("invmass", "Invariant Mass", 100, 0, 300);

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		// check conditions to ensure InvariantMass is valid
		double invmass_value = 0;
		if (delphes_tree->Electron_size == 2 && delphes_tree->Electron_Charge[0] * delphes_tree->Electron_Charge[1] == -1) 
		{
			invmass_value = sqrt(2*delphes_tree->Electron_PT[0]*delphes_tree->Electron_PT[1]*(cosh(
				delphes_tree->Electron_Eta[0] - delphes_tree->Electron_Eta[1]) - cos(delphes_tree->Electron_Phi[0] - 
				delphes_tree->Electron_Phi[1])));
		} else if (delphes_tree->Muon_size == 2 && delphes_tree->Muon_Charge[0] * delphes_tree->Muon_Charge[1] == -1) 
		{
			invmass_value = sqrt(2*delphes_tree->Muon_PT[0]*delphes_tree->Muon_PT[1]*(cosh(
				delphes_tree->Muon_Eta[0] - delphes_tree->Muon_Eta[1]) - cos(delphes_tree->Muon_Phi[0] - 
				delphes_tree->Muon_Phi[1])));

		}
		invmass->Fill(invmass_value);
	}
	
	TCanvas *invmass_canvas = new TCanvas();
	invmass_canvas->cd();
	invmass->Draw();
	invmass_canvas->SaveAs("invmass.png");
}