#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"

void plot_allFinalProcessInfo()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	TH1D *Jet_PT_hist = new TH1D("Jet_PT_hist", "Jet.pT", 60, 0, 120);
    TH1D *Electron_Pt_hist = new TH1D("Electron_Pt_hist", "Electron.pT", 60, 0, 120);

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		Jet_PT_hist->Fill(delphes_tree->Jet_PT[0]);
        Electron_Pt_hist->Fill(delphes_tree->Electron_PT[0]);
	}
	
	TCanvas *pT_canvas = new TCanvas();
	pT_canvas->cd();
	Jet_PT_hist->Draw();
    Electron_Pt_hist->Draw("same");
	pT_canvas->SaveAs("pT.png");

}