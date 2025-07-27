void FilledHistogramExample() {
  gStyle->SetOptStat(0);  // Hide stats box

  // Create canvas
  TCanvas *c = new TCanvas("c", "Overlapping Histograms", 800, 600);

  // Histogram 1: Gaussian centered at 0
  TH1F *h1 = new TH1F("h1", "Overlapping Distributions;X value;Entries", 50, -5, 10);
  for (int i = 0; i < 10000; ++i)
    h1->Fill(gRandom->Gaus(0, 1.5));

  // Histogram 2: Gaussian centered at 2
  TH1F *h2 = new TH1F("h2", "Overlapping Distributions;X value;Entries", 50, -5, 10);
  for (int i = 0; i < 10000; ++i)
    h2->Fill(gRandom->Gaus(2, 1.5));

  // Style first histogram
  h1->SetFillColorAlpha(kRed + 1, 0.5);  // semi-transparent red
  //h1->SetFillStyle(3001); // solid fill
  /*3004: diagonal lines (/)
    3005: crossed lines (X)
    3006: vertical lines (|)
    3007: horizontal lines (–)
    3008: grid*/
  h1->SetLineColor(kRed + 3);
  h1->SetLineWidth(2);

  // Style second histogram
  h2->SetFillColorAlpha(kBlue + 1, 0.5); // semi-transparent blue
  h2->SetLineColor(kBlue + 3);
  h2->SetLineWidth(2);

  // Draw both with "SAME" to overlap
  h1->Draw("HIST");
  h2->Draw("HIST SAME");

  // Add legend
  auto legend = new TLegend(0.65, 0.7, 0.88, 0.88);
  legend->AddEntry(h1, "Sample A (mean 0)", "f");
  legend->AddEntry(h2, "Sample B (mean 2)", "f");
  legend->Draw();

  // Save result
  c->SaveAs("overlapping_histograms.png");
  gApplication->Terminate(0); // Exit ROOT with code 0
}
