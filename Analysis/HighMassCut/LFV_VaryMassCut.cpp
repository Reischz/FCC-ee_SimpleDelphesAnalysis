// control cut flow
// Selection Switch
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <filesystem>
#include <unordered_map>

using namespace std;

void LFV_VaryMassCut() {
    unordered_map<TString,bool> cutFlow = {
        {"NoCut", true},
        {"ZOffShellCut", true}
    };
    cout << "Cut Flow Configuration:" << cutFlow["NoCut"] << endl;
    cout << "Z Off-Shell Cut Enabled: " << cutFlow["ZOffShellCut"] << endl;
}