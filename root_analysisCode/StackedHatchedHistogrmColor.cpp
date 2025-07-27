void StackedHatchedHistogramColor() {
  gStyle->SetOptStat(0);

  // Canvas setup
  TCanvas *c = new TCanvas("c", "Stacked + Hatched Histograms", 800, 600);

  // Histogram 1: Background A
  TH1F *h1 = new TH1F("h1", "Background A", 50, -5, 5);
  for (int i = 0; i < 5000; ++i) h1->Fill(gRandom->Gaus(-1, 1.2));
  h1->SetLineColor(kMagenta + 1);
  h1->SetFillColor(kMagenta + 1);
  h1->SetFillStyle(3004);

  // Histogram 2: Background B
  TH1F *h2 = new TH1F("h2", "Background B", 50, -5, 5);
  for (int i = 0; i < 4000; ++i) h2->Fill(gRandom->Gaus(0.5, 1.2));
  h2->SetLineColor(kOrange + 1);
  h2->SetFillColor(kOrange + 1);
  h2->SetFillStyle(3005);

  // Histogram 3: Signal
  TH1F *h3 = new TH1F("h3", "Signal", 50, -5, 5);
  for (int i = 0; i < 2000; ++i) h3->Fill(gRandom->Gaus(1.5, 0.8));
  h3->SetLineColor(kBlue + 2);
  h3->SetFillColor(kBlue + 2);
  h3->SetFillStyle(3006);

  // Stack them
  THStack *hs = new THStack("hs", "Stacked Histogram;X axis;Entries");
  hs->Add(h1);
  hs->Add(h2);
  hs->Add(h3);

  // Draw
  hs->Draw("HIST");  // do NOT use "SAME" here
  c->BuildLegend(0.65, 0.75, 0.88, 0.88);
  c->SaveAs("stacked_hatched_histograms.png");
}
