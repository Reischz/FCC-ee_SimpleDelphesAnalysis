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

void JetMass_dijetEvent()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	TH2D *hist2d = new TH2D("hist2d", "two-dimensional histogram;#Delta #Phi;Inv. Mass", 60, 0, 6, 210, 80, 300);

	TH1F *JetMass = new TH1F("JetMass", "Jet Mass", 300, 0, 300);
	TH1F *JetMomentum = new TH1F("JetMomentum", "Jet Momentum", 300, 0, 300);
	TH1F *JetEnergy = new TH1F("JetEnergy", "Jet Energy", 300, 0, 300);

	// try using btag to find higgs invariant mass
	TH1F *higgsInvM2Jet = new TH1F("higgsInvM2Jet", "Higgs Invariant Mass w/ 2 Jets", 75, 90, 240); // IGNORE

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		// check conditions to ensure InvariantMass is valid
		double invmass_value = 0;
		double higgsInvM_value = 0;
		double higgsInvM2Jet_value = 0;
		double DeltaR_value = 0;
		double DeltaPhi_value = 0;
		 
		// loop to find all Btagged jets
		int btaggedJets = 0;
		for (int i = 0; i < delphes_tree->Jet_size; i++) 
		{
			btaggedJets += delphes_tree->Jet_BTag[i];
		}

		if (btaggedJets >= 2) 
		{
			std::vector<int> btaggedJetIndex;
			for (int i = 0; i < delphes_tree->Jet_size; i++) 
			{
				if (delphes_tree->Jet_BTag[i]==1) 
				{
					btaggedJetIndex.push_back(i);
				}
			}

			double JM1 = delphes_tree->Jet_Mass[btaggedJetIndex[0]];
			double JM2 = delphes_tree->Jet_Mass[btaggedJetIndex[1]];
			JetMass->Fill(JM1);
			JetMass->Fill(JM2);
			double JP1= delphes_tree->Jet_PT[btaggedJetIndex[0]] *
				cosh(delphes_tree->Jet_Eta[btaggedJetIndex[0]]);
			double JP2 = delphes_tree->Jet_PT[btaggedJetIndex[1]] *
				cosh(delphes_tree->Jet_Eta[btaggedJetIndex[1]]);
			JetMomentum->Fill(JP1);
			JetMomentum->Fill(JP2);
			double JE1 = sqrt(pow(JM1, 2) + pow(JP1, 2));
			double JE2 = sqrt(pow(JM2, 2) + pow(JP2, 2));
			JetEnergy->Fill(JE1);
			JetEnergy->Fill(JE2);
		}

		if (higgsInvM2Jet_value > 0) 
		{
			hist2d->Fill(DeltaPhi_value, higgsInvM2Jet_value);
			higgsInvM2Jet->Fill(higgsInvM2Jet_value);
		}
	}
	
	TCanvas *higgsInvM_canvas = new TCanvas();

	TCanvas *hist2d_canvas = new TCanvas();
	hist2d_canvas->cd();
	hist2d->Draw("COLZ");
	//hist2d_canvas->SaveAs("hist2d.png");

	TCanvas *JetMass_canvas = new TCanvas();
	JetMass_canvas->cd();	
	JetMass->Draw();
	JetMass_canvas->SaveAs("JetMass.png");

	TCanvas *JetMomentum_canvas = new TCanvas();
	JetMomentum_canvas->cd();
	JetMomentum->Draw();
	JetMomentum_canvas->SaveAs("JetMomentum.png");

	TCanvas *JetEnergy_canvas = new TCanvas();
	JetEnergy_canvas->cd();
	JetEnergy->Draw();
	JetEnergy_canvas->SaveAs("JetEnergy.png");
	

	std::cout << M_PI << std::endl; // Print the value of M_PI to verify it's defined correctly
	gApplication->Terminate(0); // Exit ROOT with code 0
}