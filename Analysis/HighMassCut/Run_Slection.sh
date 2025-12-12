#!/bin/bash
#SBATCH --job-name=Sel
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=6
#SBATCH --mem=18G

FILELIST=(
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass110_Seed110/HZ4l_HLFV_1M_HMass110_Seed110_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass115_Seed115/HZ4l_HLFV_1M_HMass115_Seed115_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass120_Seed120/HZ4l_HLFV_1M_HMass120_Seed120_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass125_Seed125/HZ4l_HLFV_1M_HMass125_Seed125_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass130_Seed130/HZ4l_HLFV_1M_HMass130_Seed130_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass135_Seed135/HZ4l_HLFV_1M_HMass135_Seed135_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass140_Seed140/HZ4l_HLFV_1M_HMass140_Seed140_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/MassScan_HZ4l_HLFV/HZ4l_HLFV_1M_HMass145_Seed145/HZ4l_HLFV_1M_HMass145_Seed145_IDEA.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/HZFourLepton_1M_Seed10/HZFourLepton_1M_Seed10.root"
    "/work/project/escience/ruttho/FCC-ee_SimpleDelphesAnalysis/EventSample/ZWWFourLepton_1M_Seed30/ZWWFourLepton_1M_Seed30.root")
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
    "HZ4l_SelectionResults.root"
    "ZWW4l_SelectionResults.root"
)
for i in "${!FILELIST[@]}"; do
    echo "Processing file ${FILELIST[$i]}"
    root -l -b -q "../Z_off_shell_cut.cpp(\"${FILELIST[$i]}\", \"${RESULT[$i]}\")" &
    # Limit the number of concurrent jobs to 6
    if (( (i + 1) % 6 == 0 )); then
        wait
    fi
done
wait
echo "All selection jobs submitted."