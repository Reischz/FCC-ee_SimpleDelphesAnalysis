#include "TFile.h"
#include "TTree.h"
#include "Delphes.C"

void try_invariantMass()
{
	TFile *file = new TFile("output01.root");
	TTree *tree = (TTree*) file->Get("Delphes");

	Delphes *delphes_tree = new Delphes(tree);
	TH1F *invmass = new TH1F("invmass", "Invariant Mass", 70, 50, 120);

	// try using btag to find higgs invariant mass
	TH1F *higgsInvM = new TH1F("higgsInvM", "Higgs Invariant Mass", 70, 50, 120);

	for (Long64_t jentry=0; jentry<tree->GetEntries(); jentry++)
	{
		delphes_tree->GetEntry(jentry);
		// check conditions to ensure InvariantMass is valid
		double invmass_value = 0;
		double higgsInvM_value = 0;
		 
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
				if (delphes_tree->Jet_BTag[i]) 
				{
					btaggedJetIndex.push_back(i);
				}
			}
			higgsInvM_value = sqrt(2 * delphes_tree->Jet_PT[btaggedJetIndex[0]] * delphes_tree->Jet_PT[btaggedJetIndex[1]] *
				(cosh(delphes_tree->Jet_Eta[btaggedJetIndex[0]] - delphes_tree->Jet_Eta[btaggedJetIndex[1]]) - 
				cos(delphes_tree->Jet_Phi[btaggedJetIndex[0]] - delphes_tree->Jet_Phi[btaggedJetIndex[1]])));
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
		invmass->Fill(invmass_value);
	}
	
	TCanvas *higgsInvM_canvas = new TCanvas();
	higgsInvM_canvas->cd();
	higgsInvM->Draw();
	higgsInvM_canvas->SaveAs("higgsInvM.png");

	TCanvas *invmass_canvas = new TCanvas();
	invmass_canvas->cd();
	invmass->Draw();
	invmass_canvas->SaveAs("invmass.png");
	
	gApplication->Terminate(0); // Exit ROOT with code 0
}