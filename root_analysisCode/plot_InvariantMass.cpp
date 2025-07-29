#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"
#include <cmath>

double deltaPhi(double phi1, double phi2) {
	double deltaPhiValue = (phi1 - phi2);
	if (deltaPhiValue > M_PI) {
		deltaPhiValue = 2 * M_PI - deltaPhiValue;
	} else if (deltaPhiValue < -M_PI) {
		deltaPhiValue = 2 * M_PI + deltaPhiValue;
	}
	return deltaPhiValue;
}

void plot_InvariantMass(const char* Png_prefix="", const char* filename = "output01.root")
{
	TFile *file = new TFile(filename);	
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	THStack *InvMass = new THStack("InvMass", "Invariant Mass (Stacked); GeV; Entries");
	THStack *JetDist = new THStack("JetDist", "Jets Distribution; Number of Jets; Events");
	THStack *InvMassJet = new THStack("InvMassJet", "Invariant Mass from BTagged-Jets pair; GeV; Entries");
	TH2D *hist2d = new TH2D("hist2d", "two-dimensional histogram;#Delta #Phi;Inv. Mass", 60, 0, 6, 210, 80, 300);

	TH1F *Einvmass = new TH1F("Einvmass", "Electron Pair", 120, 0, 120);
	TH1F *Minvmass = new TH1F("Minvmass", "Muon Pair", 120, 0, 120);

	// try using btag to find higgs invariant mass
	TH1F *higgsInvM = new TH1F("higgsInvM", "Higgs Invariant Mass", 300, 0, 300);
	TH1F *higgsInvM2Jet = new TH1F("higgsInvM2Jet", "Higgs Invariant Mass w/ 2 Jets", 300, 0, 300); // IGNORE
	TH1F *BtaggedJets = new TH1F("BtaggedJets", "B-Tagged Jets", 6, -0.5, 5.5);
	TH1F *Btagged2Jets = new TH1F("Btagged2Jets", "B-Tagged Dijet", 6, -0.5, 5.5);
	TH1F *JetSize = new TH1F("JetSize", "All Jets", 6, -0.5, 5.5);

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		// check conditions to ensure InvariantMass is valid
		double higgsInvM_value = 0;
		double higgsInvM2Jet_value = 0;
		double DeltaR_value = 0;
		double DeltaPhi_value = 0;
		 
		// loop to find all Btagged jets
		int btaggedJets = 0;
		int JetSize_value = delphes_tree->Jet_size;
		JetSize->Fill(JetSize_value);
		for (int i = 0; i < JetSize_value; i++) 
		{
			btaggedJets += delphes_tree->Jet_BTag[i];
		}
		BtaggedJets->Fill(btaggedJets);
		if (btaggedJets == 2) 
		{
			std::vector<int> btaggedJetIndex;
			for (int i = 0; i < JetSize_value; i++) 
			{
				if (delphes_tree->Jet_BTag[i]==1) 
				{
					btaggedJetIndex.push_back(i);
				}
			}

			higgsInvM_value = sqrt(2 * delphes_tree->Jet_PT[btaggedJetIndex[0]] * delphes_tree->Jet_PT[btaggedJetIndex[1]] *
				(cosh(delphes_tree->Jet_Eta[btaggedJetIndex[0]] - delphes_tree->Jet_Eta[btaggedJetIndex[1]]) - 
				cos(deltaPhi(delphes_tree->Jet_Phi[btaggedJetIndex[0]], delphes_tree->Jet_Phi[btaggedJetIndex[1]]))));
			if (JetSize_value == 2) 
			{
				Btagged2Jets->Fill(btaggedJets);
				DeltaPhi_value = deltaPhi(delphes_tree->Jet_Phi[0], delphes_tree->Jet_Phi[1]);
				DeltaR_value = sqrt(pow(delphes_tree->Jet_Eta[0] - delphes_tree->Jet_Eta[1], 2) + 
				pow(deltaPhi(delphes_tree->Jet_Phi[0], delphes_tree->Jet_Phi[1]), 2));

				higgsInvM2Jet_value = sqrt(2 * delphes_tree->Jet_PT[0] * delphes_tree->Jet_PT[1] *
					(cosh(delphes_tree->Jet_Eta[0] - delphes_tree->Jet_Eta[1]) - 
					cos(deltaPhi(delphes_tree->Jet_Phi[0], delphes_tree->Jet_Phi[1]))));
			}
		}


		if (delphes_tree->Electron_size == 2 && delphes_tree->Electron_Charge[0] * delphes_tree->Electron_Charge[1] == -1) 
		{
			double invmass_value = sqrt(2*delphes_tree->Electron_PT[0]*delphes_tree->Electron_PT[1]*(cosh(
						delphes_tree->Electron_Eta[0] - delphes_tree->Electron_Eta[1]) - cos(deltaPhi(delphes_tree->Electron_Phi[0], 
						delphes_tree->Electron_Phi[1]))));
			Einvmass->Fill(invmass_value);
		} else if (delphes_tree->Muon_size == 2 && delphes_tree->Muon_Charge[0] * delphes_tree->Muon_Charge[1] == -1) 
		{
			double invmass_value = sqrt(2*delphes_tree->Muon_PT[0]*delphes_tree->Muon_PT[1]*(cosh(
				delphes_tree->Muon_Eta[0] - delphes_tree->Muon_Eta[1]) - cos(deltaPhi(delphes_tree->Muon_Phi[0], 
				delphes_tree->Muon_Phi[1]))));
			Minvmass->Fill(invmass_value);
		}
		if (higgsInvM_value > 0) 
		{
			higgsInvM->Fill(higgsInvM_value);
		}
		if (higgsInvM2Jet_value > 0) 
		{
			hist2d->Fill(DeltaPhi_value, higgsInvM2Jet_value);
			higgsInvM2Jet->Fill(higgsInvM2Jet_value);
		}
	}
	
	TCanvas *c1 = new TCanvas();
	c1->cd();
	higgsInvM->SetLineColor(kBlue);
	higgsInvM->SetFillColorAlpha(kBlue+1, 0.3); // semi-transparent
	InvMass->Add(higgsInvM);
	Einvmass->SetLineColor(kMagenta);
	Einvmass->SetFillColorAlpha(kMagenta+1, 0.3); // semi-transparent
	InvMass->Add(Einvmass);
	Minvmass->SetLineColor(kOrange);
	Minvmass->SetFillColorAlpha(kOrange+1, 0.3);
	InvMass->Add(Minvmass);
	InvMass->Draw();
	// Instead of c1->BuildLegend(), create custom legend
	TLegend *legend = new TLegend(0.65, 0.75, 0.88, 0.88);
	legend->AddEntry(higgsInvM, Form("Jet (%d)", (int)higgsInvM->GetEntries()), "f");
	legend->AddEntry(Einvmass, Form("Electron (%d)", (int)Einvmass->GetEntries()), "f");
	legend->AddEntry(Minvmass, Form("Muon (%d)", (int)Minvmass->GetEntries()), "f");
	legend->Draw();
	c1->SaveAs(Form("%sInvariantMass.png", Png_prefix));
	c1->Clear();
	legend->Clear();
	gPad->SetLogy(0);
	JetSize->SetLineColor(kBlue);
	JetSize->SetFillColorAlpha(kBlue+1, 0.3); // semi-transparent
	JetDist->Add(JetSize);
	BtaggedJets->SetLineColor(kRed);
	BtaggedJets->SetFillColorAlpha(kRed+1, 0.3);
	JetDist->Add(BtaggedJets);
	Btagged2Jets->SetLineColor(kGreen);
	Btagged2Jets->SetFillColorAlpha(kGreen+1, 0.3);
	JetDist->Add(Btagged2Jets);
	JetDist->Draw("nostack");
	//TLegend *legend = new TLegend(0.65, 0.75, 0.88, 0.88);
	legend->AddEntry(JetSize, Form("All Jets (%d)", (int)JetSize->GetEntries()), "f");
	legend->AddEntry(BtaggedJets, Form("B-Tagged Jets (%d)", (int)BtaggedJets->GetEntries()), "f");
	legend->AddEntry(Btagged2Jets, Form("B-Tagged Dijet (%d)", (int)Btagged2Jets->GetEntries()), "f");
	legend->Draw();
	c1->SaveAs(Form("%sJetDistribution.png", Png_prefix));
	c1->Clear();
	legend->Clear();
	higgsInvM->SetLineColor(kBlue);
	higgsInvM->SetFillColorAlpha(kBlue+1, 0.3); // semi-transparent
	InvMassJet->Add(higgsInvM);
	InvMassJet->Add(higgsInvM2Jet);
	higgsInvM2Jet->SetLineColor(kRed);
	higgsInvM2Jet->SetFillColorAlpha(kRed+1, 0.3); // semi-transparent
	InvMassJet->Draw("nostack");
	legend->AddEntry(higgsInvM, Form("All Jets (%d)", (int)higgsInvM->GetEntries()), "f");
	legend->AddEntry(higgsInvM2Jet, Form("Dijet (%d)", (int)higgsInvM2Jet->GetEntries()), "f");
	legend->Draw();
	c1->SaveAs(Form("%sInvariantMass_JetOnly.png", Png_prefix));

	
	gApplication->Terminate(0); // Exit ROOT with code 0
}