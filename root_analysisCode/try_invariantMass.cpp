#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"

void plot_allFinalProcessInfo()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	TH1D *invmass = new TH1D("invmass", "Invariant Mass", 60, 0, 120);

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		// check conditions to ensure InvariantMass is valid
		if (delphes_tree->Electron_size == 2)
		{
			
		}
		invmass->Fill(delphes_tree->InvariantMass);

	}
	
	TCanvas *invmass_canvas = new TCanvas();
	invmass_canvas->cd();
	invmass->Draw();
	invmass_canvas->SaveAs("invmass.png");

}