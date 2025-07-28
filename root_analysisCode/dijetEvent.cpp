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

void dijetEvent()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	THStack *InvMass = new THStack("InvMass", "Invariant Mass; GeV; Entries");
	THStack *JetDist = new THStack("JetDist", "Jets Distribution; Number of Jets; Events");
	TH2D *hist2d = new TH2D("hist2d", "two-dimensional histogram;#Delta #Phi;Inv. Mass", 60, 0, 6, 210, 80, 300);

	TH1F *Einvmass = new TH1F("Einvmass", "Electron Pair", 70, 50, 120);
	TH1F *Minvmass = new TH1F("Minvmass", "Muon Pair", 70, 50, 120);

	// try using btag to find higgs invariant mass
	TH1F *higgsInvM = new TH1F("higgsInvM", "Higgs Invariant Mass", 90, 50, 240);
	TH1F *higgsInvM2Jet = new TH1F("higgsInvM2Jet", "Higgs Invariant Mass w/ 2 Jets", 75, 90, 240); // IGNORE
	TH1F *BtaggedJets = new TH1F("BtaggedJets", "B-Tagged Jets", 5, -0.5, 4.5);
	TH1F *Btagged2Jets = new TH1F("Btagged2Jets", "B-Tagged Dijet", 5, -0.5, 4.5);
	TH1F *JetSize = new TH1F("JetSize", "All Jets", 5, -0.5, 4.5);

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
	InvMass->Add(higgsInvM);
	InvMass->Add(Einvmass);
	InvMass->Add(Minvmass);
	InvMass->Draw();
	c1->BuildLegend(0.65, 0.75, 0.88, 0.88);
	gPad->SetLogy();
	c1->SaveAs("InvariantMass.png");
	c1->Clear();
	gPad->SetLogy(0);
	JetDist->Add(JetSize);
	JetDist->Add(BtaggedJets);
	JetDist->Add(Btagged2Jets);
	JetDist->Draw();
	c1->BuildLegend(0.65, 0.75, 0.88, 0.88);
	c1->SaveAs("JetDistribution.png");
	c1->Clear();
	higgsInvM2Jet->SetTitle("Invariant Mass with 2 Jets; GeV; Entries");
	higgsInvM2Jet->Draw();
	c1->SaveAs("BTagged2JetInvMass.png");

	
	gApplication->Terminate(0); // Exit ROOT with code 0
}