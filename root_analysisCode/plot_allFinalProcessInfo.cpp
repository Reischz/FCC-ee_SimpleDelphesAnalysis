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
	TH1D *Muon_Pt_hist = new TH1D("Muon_Pt_hist", "Muon.pT", 60, 0, 120);
	TH1F *Jet_size_hist = new TH1F("Jet_size_hist", "Jet@size", 5, -0.5, 4.5);

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		Jet_PT_hist->Fill(delphes_tree->Jet_PT[0]);
        Electron_Pt_hist->Fill(delphes_tree->Electron_PT[0]);
		Muon_Pt_hist->Fill(delphes_tree->Muon_PT[0]);
		Jet_size_hist->Fill(delphes_tree->Jet_size);
	}
	
	TCanvas *pT_canvas = new TCanvas();
	pT_canvas->cd();
	Jet_PT_hist->Draw();
	Jet_PT_hist->SetStats(0);  // Disable default stats box
	//Jet_PT_hist->SetFillColor(kBlue);
    Electron_Pt_hist->Draw("same");
	Electron_Pt_hist->SetLineColor(kRed);
	Electron_Pt_hist->SetStats(0);  // Disable default stats box
	//Electron_Pt_hist->SetFillColor(kRed);
	Muon_Pt_hist->SetStats(0);  // Disable default stats box
	//Muon_Pt_hist->SetFillColor(kGreen);
	Muon_Pt_hist->Draw("same");
	Muon_Pt_hist->SetLineColor(kGreen);
	pT_canvas->BuildLegend();
	pT_canvas->SetTitle("pT Distributions");
	pT_canvas->Update();

	pT_canvas->SaveAs("pT.png");

	TCanvas *Jet_size_canvas = new TCanvas();
	Jet_size_canvas->cd();
	Jet_size_hist->Draw();
	Jet_size_canvas->SaveAs("Jet_size.png");

}