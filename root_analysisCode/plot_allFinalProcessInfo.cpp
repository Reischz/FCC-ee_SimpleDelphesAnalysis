#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"

void plot_allFinalProcessInfo(const char* Png_prefix="", const char* filename = "output01.root")
{
	TFile *file = new TFile(filename);	
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	THStack *PTStackedHist = new THStack("PTStackedHist", "Transverse Momentum");
	THStack *SizeStackedHist = new THStack("SizeStackedHist", "Number of Particles/Jet");
	THStack *EtaStackedHist = new THStack("EtaStackedHist", "Eta");
	THStack *PhiStackedHist = new THStack("PhiStackedHist", "Phi");
	THStack *ChargeStackedHist = new THStack("ChargeStackedHist", "Charge");
	TH1D *Jet_PT_hist = new TH1D("Jet_PT_hist", "Jet.pT", 60, 0, 120);
    TH1D *Electron_Pt_hist = new TH1D("Electron_Pt_hist", "Electron.pT", 60, 0, 120);
	TH1D *Muon_Pt_hist = new TH1D("Muon_Pt_hist", "Muon.pT", 60, 0, 120);
	TH1D *Jet_size_hist = new TH1D("Jet_size_hist", "Jet@size", 5, -0.5, 4.5);
	TH1D *Electron_size_hist = new TH1D("Electron_size_hist", "Electron@size", 5, -0.5, 4.5);
	TH1D *Muon_size_hist = new TH1D("Muon_size_hist", "Muon@size", 5, -0.5, 4.5);
	TH1D *Jet_Eta_hist = new TH1D("Jet_Eta_hist", "Jet.Eta", 60, -3, 3);
	TH1D *Electron_Eta_hist = new TH1D("Electron_Eta_hist", "Electron.Eta", 60, -3, 3);
	TH1D *Muon_Eta_hist = new TH1D("Muon_Eta_hist", "Muon.Eta", 60, -3, 3);
	TH1D *Jet_Phi_hist = new TH1D("Jet_Phi_hist", "Jet.Phi", 100, -5, 5);
	TH1D *Electron_Phi_hist = new TH1D("Electron_Phi_hist", "Electron.Phi", 100, -5, 5);
	TH1D *Muon_Phi_hist = new TH1D("Muon_Phi_hist", "Muon.Phi", 100, -5, 5);
	TH1D *Electron_Charge_hist = new TH1D("Electron_Charge_hist", "Electron.Charge", 3, -1.5, 1.5);
	TH1D *Muon_Charge_hist = new TH1D("Muon_Charge_hist", "Muon.Charge", 3, -1.5, 1.5);
	std::vector<TH1D*> histograms = {
		Jet_PT_hist, Electron_Pt_hist, Muon_Pt_hist,
		Jet_size_hist, Electron_size_hist, Muon_size_hist,
		Jet_Eta_hist, Electron_Eta_hist, Muon_Eta_hist,
		Jet_Phi_hist, Electron_Phi_hist, Muon_Phi_hist,
		Electron_Charge_hist, Muon_Charge_hist
	};


	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		int JetSize = delphes_tree->Jet_size;
		if (JetSize > 0) {
			for(int i = 0; i < JetSize; ++i) {
				Jet_size_hist->Fill(JetSize);
				Jet_PT_hist->Fill(delphes_tree->Jet_PT[i]);
				Jet_Eta_hist->Fill(delphes_tree->Jet_Eta[i]);
				Jet_Phi_hist->Fill(delphes_tree->Jet_Phi[i]);
			}
		}

		int ElectronSize = delphes_tree->Electron_size;
		if (ElectronSize > 0) {
			for(int i = 0; i < ElectronSize; ++i) {
				Electron_size_hist->Fill(ElectronSize);
				Electron_Pt_hist->Fill(delphes_tree->Electron_PT[i]);
				Electron_Eta_hist->Fill(delphes_tree->Electron_Eta[i]);
				Electron_Phi_hist->Fill(delphes_tree->Electron_Phi[i]);
				Electron_Charge_hist->Fill(delphes_tree->Electron_Charge[i]);
			}
		}
		int MuonSize = delphes_tree->Muon_size;
		if (MuonSize > 0) {
			for(int i = 0; i < MuonSize; ++i) {
				Muon_size_hist->Fill(MuonSize);
				Muon_Pt_hist->Fill(delphes_tree->Muon_PT[i]);
				Muon_Eta_hist->Fill(delphes_tree->Muon_Eta[i]);
				Muon_Phi_hist->Fill(delphes_tree->Muon_Phi[i]);
				Muon_Charge_hist->Fill(delphes_tree->Muon_Charge[i]);
			}
		}
	}
	
	TCanvas *c1 = new TCanvas("c1", "Canvas Title");
	c1->cd();
	//double maxY = std::max({Jet_PT_hist->GetMaximum(), Electron_Pt_hist->GetMaximum(), Muon_Pt_hist->GetMaximum()});
	//Jet_PT_hist->SetMaximum(maxY * 1.2);

	std::vector<Color_t> HistogramColors = {kBlue, kMagenta, kOrange};
	for(int j=0;j<3;j++){
		histograms[j]->SetLineColor(HistogramColors[j]);
		histograms[j]->SetFillColorAlpha(HistogramColors[j]+1, 0.3); // semi-transparent
		PTStackedHist->Add(histograms[j]);

		histograms[j+3]->SetLineColor(HistogramColors[j]);
		histograms[j+3]->SetFillColorAlpha(HistogramColors[j]+1, 0.3);
		SizeStackedHist->Add(histograms[j+3]);

		histograms[j+6]->SetLineColor(HistogramColors[j]);
		histograms[j+6]->SetFillColorAlpha(HistogramColors[j]+1, 0.3);
		EtaStackedHist->Add(histograms[j+6]);

		histograms[j+9]->SetLineColor(HistogramColors[j]);
		histograms[j+9]->SetFillColorAlpha(HistogramColors[j]+1, 0.3);
		PhiStackedHist->Add(histograms[j+9]);

		if (j<2) { // Only for Electron and Muon
			histograms[j+12]->SetLineColor(HistogramColors[j+1]);
			histograms[j+12]->SetFillColorAlpha(HistogramColors[j+1]+1, 0.3);
			ChargeStackedHist->Add(histograms[j+12]);
		}
	}

	PTStackedHist->Draw();
	PTStackedHist->SetTitle("Transverse Momentum; GeV; Entries");
	c1->BuildLegend(0.65, 0.75, 0.88, 0.88);
	c1->SaveAs(Form("%sPTStackedHist.png", Png_prefix));
	c1->Clear();
	SizeStackedHist->Draw();
	SizeStackedHist->SetTitle("Number of Particles/Jet; ; Entries");
	c1->BuildLegend(0.65, 0.75, 0.88, 0.88);
	c1->SaveAs(Form("%sSizeStackedHist.png", Png_prefix));
	c1->Clear();
	EtaStackedHist->Draw();
	EtaStackedHist->SetTitle("Pseudo-Rapidity; ; Entries");
	c1->BuildLegend(0.65, 0.75, 0.88, 0.88);
	c1->SaveAs(Form("%sEtaStackedHist.png", Png_prefix));
	c1->Clear();
	PhiStackedHist->Draw();
	PhiStackedHist->SetTitle("Phi; Rad; Entries");
	c1->BuildLegend(0.65, 0.75, 0.88, 0.88);
	c1->SaveAs(Form("%sPhiStackedHist.png", Png_prefix));
	c1->Clear();
	ChargeStackedHist->Draw();
	ChargeStackedHist->SetTitle("Charge; ; Entries");
	c1->BuildLegend(0.65, 0.75, 0.88, 0.88);
	c1->SaveAs(Form("%sChargeStackedHist.png", Png_prefix));
	gApplication->Terminate(0); // Exit ROOT with code 0
}
