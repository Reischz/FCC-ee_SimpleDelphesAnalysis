#!/bin/bash
#SBATCH --job-name=offsel
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
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
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass150_Seed150/HZ4l_HLFV_1M_HMass150_Seed150_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass115_Seed155/HZ4l_HLFV_1M_HMass115_Seed155_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass120_Seed160/HZ4l_HLFV_1M_HMass160_Seed160_IDEA.root"
    '/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/zh_ll_tautau.root'
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/zh_ll_ww.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/zz_ll_tautau.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/ZWWFourLepton_1M_Seed30/ZWWFourLepton_1M_Seed30.root"
    )
NUM_FILES=${#FILELIST[@]}
rm -rf SelectionResults_Ofs
mkdir SelectionResults_Ofs
cd SelectionResults_Ofs
RESULT_NAME=(
    "HLFV_150GeV"
    "HLFV_155GeV"
    "HLFV_160GeV"
    "ZHTaTa"
    "ZHWW"
    "ZZTaTa"
    "ZWW4l"
)

for i in "${!FILELIST[@]}"; do
    echo "Processing file ${FILELIST[$i]}"
    root -l -b -q "../Z_off_shell_cut.cpp(\"${FILELIST[$i]}\", \"${RESULT_NAME[$i]}_SelectionResults.root\", \"${RESULT_NAME[$i]}_AdditionalTree.root\")" &
    # Limit the number of concurrent jobs to 6
    # if (( (i + 1) % 6 == 0 )); then
    #     wait
    # fi
done
wait
echo "All selection jobs submitted."