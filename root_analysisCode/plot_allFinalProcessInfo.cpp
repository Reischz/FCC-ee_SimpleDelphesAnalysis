!#/bin/env root -l -b -q

#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"

void plot_allFinalProcessInfo()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	TH1D *Jet_PT_hist = new TH1D("Jet_PT_hist", "Jet.pT", 60, 0, 120);

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		Jet_PT_hist->Fill(delphes_tree->Jet_PT[0]);
	}
	
	TCanvas *pT_canvas = new TCanvas();
	pT_canvas->cd();
	Jet_PT_hist->Draw();
	pT_canvas->SaveAs("pT.png");

}