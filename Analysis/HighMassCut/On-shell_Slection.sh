#!/bin/bash
#SBATCH --job-name=onSel
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpugpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=6
#SBATCH --mem=18G

# Source the CMS environment
source /work/app/cms/cmsset_default.sh
function cmsset() {
  cd ~/binary/CMSSW_14_1_0_pre5/src && source /work/app/share_env/hepsw.sh && cmsenv && cd -
}
cmsset

FILELIST=(
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass110_Seed110/HZ4l_HLFV_1M_HMass110_Seed110_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass115_Seed115/HZ4l_HLFV_1M_HMass115_Seed115_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass120_Seed120/HZ4l_HLFV_1M_HMass120_Seed120_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass125_Seed125/HZ4l_HLFV_1M_HMass125_Seed125_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass130_Seed130/HZ4l_HLFV_1M_HMass130_Seed130_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass135_Seed135/HZ4l_HLFV_1M_HMass135_Seed135_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass140_Seed140/HZ4l_HLFV_1M_HMass140_Seed140_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass145_Seed145/HZ4l_HLFV_1M_HMass145_Seed145_IDEA.root"
    '/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/Prelim_sample/zh_ll_tautau.root'
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/Prelim_sample/zh_ll_ww.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/Prelim_sample/zz_ll_tautau.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/ZWWFourLepton_1M_Seed30/ZWWFourLepton_1M_Seed30.root"
    )
NUM_FILES=${#FILELIST[@]}

rm -rf SelectionResults/OnShellCut
mkdir -p SelectionResults/OnShellCut
cd SelectionResults/OnShellCut
RESULT_NAME=(
    "HLFV_110GeV"
    "HLFV_115GeV"
    "HLFV_120GeV"
    "HLFV_125GeV"
    "HLFV_130GeV"
    "HLFV_135GeV"
    "HLFV_140GeV"
    "HLFV_145GeV"
    "ZHTaTa"
    "ZHWW"
    "ZZTaTa"
    "ZWW4l"
)

IsThisBackGround=(
    false
    false
    false
    false
    false
    false
    false
    false
    true
    true
    true
    true
)

for i in "${!FILELIST[@]}"; do
    echo "Processing file ${FILELIST[$i]}"
    root -l -b -q "../../Normal_on_shell_cut.cpp(false, ${IsThisBackGround[$i]}, \"${FILELIST[$i]}\", \"${RESULT_NAME[$i]}_SelectionResults.root\", \"${RESULT_NAME[$i]}_AdditionalTree.root\")" &
    # Limit the number of concurrent jobs to 6
    # if (( (i + 1) % 6 == 0 )); then
    #     wait
    # fi
done
wait
echo "All selection jobs submitted."