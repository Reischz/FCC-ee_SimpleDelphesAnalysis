#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"

void example()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	TH1D *hist = new TH1D("hist", "jet pT", 60, 0, 120);

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		hist->Fill(delphes_tree->Jet_PT[0]);
	}
	
	TCanvas *canvas = new TCanvas();
	canvas->cd();
	hist->Draw();
	canvas->SaveAs("hist.png");

}
