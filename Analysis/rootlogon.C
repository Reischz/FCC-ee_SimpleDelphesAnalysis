void rootlogon() {
    // --- IMPORTANT ---
    // Replace this path with the path to YOUR Delphes installation directory
    const char* delphes_path = "/work/app/delphes/3.5.0";

    // Add the Delphes directory to the include path so ROOT can find headers
    gSystem->AddIncludePath(TString::Format("-I%s", delphes_path));

    // Load the Delphes library
    gROOT->ProcessLine(TString::Format(".L %s/libDelphes.so", delphes_path));

    std::cout << "+++ Delphes environment configured! +++" << std::endl;
}