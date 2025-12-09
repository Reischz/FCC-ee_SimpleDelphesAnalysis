#include <TFile.h>
#include <TGraph.h>
#include <TRandom3.h>

void save_scatter_graph() {
    // 1. Create vectors to hold your data (easier than arrays)
    const int n_points = 100;
    double x_vals[n_points];
    double y_vals[n_points];

    TRandom3 randGen(0);
    for (int i = 0; i < n_points; ++i) {
        x_vals[i] = i;
        y_vals[i] = randGen.Gaus(i, 5); // Random scatter
    }

    // 2. Create the TGraph
    TGraph *graph = new TGraph(n_points, x_vals, y_vals);
    
    // IMPORTANT: Set Name and Title for the file
    graph->SetName("my_scatter_plot"); 
    graph->SetTitle("PT vs MET;Lepton PT [GeV];Missing ET [GeV]");

    // Optional: Style it to look like points (Scatter)
    graph->SetMarkerStyle(20); // Full circle
    graph->SetMarkerColor(kRed);

    // 3. Save to File
    TFile *f_out = new TFile("scatter_graph.root", "RECREATE");
    
    // Write it!
    graph->Write(); 

    f_out->Close();
    delete f_out;
    delete graph;
}