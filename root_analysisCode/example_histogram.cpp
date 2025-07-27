void example_histogram() {
  // Load ROOT
  gStyle->SetOptStat(0);  // Turn off stats box

  // Create canvas
  TCanvas *c1 = new TCanvas("c1", "Filled Histogram", 800, 600);

  // Create histogram
  TH1F *h = new TH1F("h", "Random Gaussian;X value;Entries", 50, -5, 5);

  // Fill with Gaussian random numbers
  for (int i = 0; i < 10000; ++i)
    h->Fill(gRandom->Gaus(0, 1));

  // Styling: fill color and line color
  h->SetFillColorAlpha(kAzure + 7, 0.5);  // semi-transparent fill
  h->SetLineColor(kBlue + 3);             // darker border
  h->SetLineWidth(2);

  // Draw
  h->Draw("HIST");

  // Save
  c1->SaveAs("histogram_filled.pdf");
}
