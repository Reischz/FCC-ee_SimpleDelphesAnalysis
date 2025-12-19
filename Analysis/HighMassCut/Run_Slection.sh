#!/bin/bash
#SBATCH --job-name=Sel
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
    '/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/zh_ll_tautau.root'
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/zh_ll_ww.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/Analysis/HighMassCut/zz_ll_tautau.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/ZWWFourLepton_1M_Seed30/ZWWFourLepton_1M_Seed30.root"
    )
NUM_FILES=${#FILELIST[@]}
rm -rf SelectionResults
mkdir SelectionResults
cd SelectionResults
RESULT=(
    "HLFV_110GeV_SelectionResults.root"
    "HLFV_115GeV_SelectionResults.root"
    "HLFV_120GeV_SelectionResults.root"
    "HLFV_125GeV_SelectionResults.root"
    "HLFV_130GeV_SelectionResults.root"
    "HLFV_135GeV_SelectionResults.root"
    "HLFV_140GeV_SelectionResults.root"
    "HLFV_145GeV_SelectionResults.root"
    "ZHTaTa_SelectionResults.root"
    "ZHWW_SelectionResults.root"
    "ZZTaTa_SelectionResults.root"
    "ZWW4l_SelectionResults.root"
)
TREERESULT=(
    "HLFV_110GeV_AdditionalTree.root"
    "HLFV_115GeV_AdditionalTree.root"
    "HLFV_120GeV_AdditionalTree.root"
    "HLFV_125GeV_AdditionalTree.root"
    "HLFV_130GeV_AdditionalTree.root"
    "HLFV_135GeV_AdditionalTree.root"
    "HLFV_140GeV_AdditionalTree.root"
    "HLFV_145GeV_AdditionalTree.root"
    "ZHTaTa_AdditionalTree.root"
    "ZHWW_AdditionalTree.root"
    "ZZTaTa_AdditionalTree.root"
    "ZWW4l_AdditionalTree.root"
)
# for i in "${!FILELIST[@]}"; do
#     echo "Processing file ${FILELIST[$i]}"
#     root -l -b -q "../Z_off_shell_cut.cpp(\"${FILELIST[$i]}\", \"${RESULT[$i]}\", \"${TREERESULT[$i]}\")" &
#     # Limit the number of concurrent jobs to 6
#     if (( (i + 1) % 6 == 0 )); then
#         wait
#     fi
# done
# wait
# echo "All selection jobs submitted."

rm -rf SelectionResults
mkdir SelectionResults
cd SelectionResults

# 1. PRE-COMPILE STEP
# We run ROOT once on the first file just to generate the .so library.
# This avoids the 6-way race condition.
echo "Pre-compiling macro..."
root -l -b -q "../Z_off_shell_cut.cpp+(\"${FILELIST[0]}\", \"${RESULT[0]}\", \"${TREERESULT[0]}\")"

# 2. PREPARE TASK LIST
CMD_FILE="task_list.txt"
> $CMD_FILE

for i in "${!FILELIST[@]}"; do
    # Use the '+' again; ROOT will see it's already compiled and just load the .so
    echo "root -l -b -q \"../Z_off_shell_cut.cpp+(\\\"${FILELIST[$i]}\\\", \\\"${RESULT[$i]}\\\", \\\"${TREERESULT[$i]}\\\")\"" >> $CMD_FILE
done

echo "Submitting parallel jobs..."
# 3. RUN PARALLEL
tr '\n' '\0' < $CMD_FILE | xargs -0 -P $SLURM_CPUS_PER_TASK -I % sh -c %

rm $CMD_FILE
echo "All selection jobs completed successfully."