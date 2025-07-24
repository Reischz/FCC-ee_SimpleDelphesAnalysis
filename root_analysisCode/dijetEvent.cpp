#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"

void dijetEvent()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	TH2D *hist2d = new TH2D("hist2d", "two-dimensional histogram;#Delta R;Inv. Mass", 30, 2, 5, 160, 80, 240);

	TH1F *invmass = new TH1F("invmass", "Invariant Mass", 70, 50, 120);

	// try using btag to find higgs invariant mass
	TH1F *higgsInvM = new TH1F("higgsInvM", "Higgs Invariant Mass", 90, 50, 240);
	TH1F *higgsInvM2Jet = new TH1F("higgsInvM2Jet", "Higgs Invariant Mass w/ 2 Jets", 75, 90, 240); // IGNORE

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		// check conditions to ensure InvariantMass is valid
		double invmass_value = 0;
		double higgsInvM_value = 0;
		double higgsInvM2Jet_value = 0;
		double DeltaR_value = 0;
		 
		// loop to find all Btagged jets
		int btaggedJets = 0;
		for (int i = 0; i < delphes_tree->Jet_size; i++) 
		{
			btaggedJets += delphes_tree->Jet_BTag[i];
		}

		if (btaggedJets == 2) 
		{
			std::vector<int> btaggedJetIndex;
			for (int i = 0; i < delphes_tree->Jet_size; i++) 
			{
				if (delphes_tree->Jet_BTag[i]==1) 
				{
					btaggedJetIndex.push_back(i);
				}
			}
			higgsInvM_value = sqrt(2 * delphes_tree->Jet_PT[btaggedJetIndex[0]] * delphes_tree->Jet_PT[btaggedJetIndex[1]] *
				(cosh(delphes_tree->Jet_Eta[btaggedJetIndex[0]] - delphes_tree->Jet_Eta[btaggedJetIndex[1]]) - 
				cos(delphes_tree->Jet_Phi[btaggedJetIndex[0]] - delphes_tree->Jet_Phi[btaggedJetIndex[1]])));
			if (delphes_tree->Jet_size == 2) 
			{
				DeltaR_value = sqrt(pow(delphes_tree->Jet_Eta[0] - delphes_tree->Jet_Eta[1], 2) + 
					pow(delphes_tree->Jet_Phi[0] - delphes_tree->Jet_Phi[1], 2));
				higgsInvM2Jet_value = sqrt(2 * delphes_tree->Jet_PT[0] * delphes_tree->Jet_PT[1] *
					(cosh(delphes_tree->Jet_Eta[0] - delphes_tree->Jet_Eta[1]) - 
					cos(delphes_tree->Jet_Phi[0] - delphes_tree->Jet_Phi[1])));
			}
		}


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
		if (higgsInvM_value > 0) 
		{
			higgsInvM->Fill(higgsInvM_value);
		}
		if (invmass_value > 0) 
		{
			invmass->Fill(invmass_value);
		}
		if (higgsInvM2Jet_value > 0) 
		{
			hist2d->Fill(DeltaR_value, higgsInvM2Jet_value);
			higgsInvM2Jet->Fill(higgsInvM2Jet_value);
		}
	}
	
	TCanvas *higgsInvM_canvas = new TCanvas();
	higgsInvM_canvas->cd();
	higgsInvM->Draw();
	higgsInvM_canvas->SaveAs("higgsInvM.png");

	TCanvas *invmass_canvas = new TCanvas();
	invmass_canvas->cd();
	invmass->Draw();
	invmass_canvas->SaveAs("invmass.png");

	TCanvas *higgsInvM2Jet_canvas = new TCanvas();
	higgsInvM2Jet_canvas->cd();
	higgsInvM2Jet->Draw();
	higgsInvM2Jet_canvas->SaveAs("higgsInvM2Jet.png");

	TCanvas *hist2d_canvas = new TCanvas();
	hist2d_canvas->cd();
	//hist2d->Draw("COLZ");
	hist2d->Draw();
	hist2d_canvas->SaveAs("hist2d.png");
	
	gApplication->Terminate(0); // Exit ROOT with code 0
}