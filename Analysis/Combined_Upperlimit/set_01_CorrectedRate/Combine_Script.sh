#!/bin/bash
#SBATCH --job-name=combine_bg
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=6
#SBATCH --mem=12G

# Source the CMS environment
source /work/app/cms/cmsset_default.sh
function cmsset() {
  cd ~/binary/CMSSW_14_1_0_pre5/src && source /work/app/share_env/hepsw.sh && cmsenv && cd -
}
cmsset

# Median Signal Srength Upper Limit Calculation using HybridNew Method
HZ4l_Yield=(
    0.017305039
    0.010309742000000002
    0.005602391000000001
    0.0027102930000000003
    0.001132785
    0.00041395600000000005
    0.00011188000000000002
    2.5173000000000004e-05
)

ZWW4l_Yield=(
    0.016611543 
    0.009684908999999998 
    0.005155956
    0.002580669
    0.001038726
    0.00033099299999999997
    0.00010763999999999999
    1.8837e-05
)
LFV_Yield=(0.617264 0.611064 0.604286 0.597715 0.591248 0.583566 0.575209 0.569374)

Mass_Points=(110 115 120 125 130 135 140 145)
LUMI=1

COUNT=${#Mass_Points[@]}
# Expected Limit Band Calculations
for ((mass_index=0; mass_index<COUNT; mass_index++ )); do
    mkdir -p Combine_Mass_${Mass_Points[mass_index]}
    cd Combine_Mass_${Mass_Points[mass_index]}

    # Prepare Data Card
    cp ../Data_Card_Combine.dat .
    sed -i "s/XXXX/${LFV_Yield[mass_index]}/g" Data_Card_Combine.dat
    sed -i "s/YYYY/${HZ4l_Yield[mass_index]}/g" Data_Card_Combine.dat
    sed -i "s/ZZZZ/${ZWW4l_Yield[mass_index]}/g" Data_Card_Combine.dat
    CARD=Data_Card_Combine_mh${Mass_Points[mass_index]}.dat
    mv Data_Card_Combine.dat ${CARD}

    # Create the workspace
    for quantile in -1 0.5 0.84 0.16 0.975 0.025; do
        if [ $quantile == -1 ]; then
            echo "Calculating Observed Limit"
            combine -M HybridNew --LHCmode LHC-limits ${CARD} --verbose 1 --saveHybridResult -s ${LUMI} \
                    --saveToys -T 5000 --setParameters lumiscale=${LUMI} --mass ${Mass_Points[mass_index]} &
        else
            echo "Calculating Expected Limit for quantile ${quantile}"
            combine -M HybridNew --LHCmode LHC-limits ${CARD} --verbose 1 --saveHybridResult -s ${LUMI} \
                    --expectedFromGrid $quantile --saveToys -T 5000 --setParameters lumiscale=${LUMI} \
                    --mass ${Mass_Points[mass_index]} &
        fi
    done
    wait
    cd ..
done
echo "All combine jobs Completed."

# Plotting the Test Statistic Distributions and Limits
# python3 /work/home/vwachira/CMSSW_14_1_0_pre5/src/HiggsAnalysis/CombinedLimit/test/plotTestStatCLs.py --input higgsCombineTest.HybridNew.mH120.${LUMI}.root --poi r --val all --mass 120

# Median Signal Strength Upper Limit Calculation without Systematics
# combine -M HybridNew --LHCmode LHC-limits no-bkg-test-nosyst.txt -n _nosyst --verbose 1 --saveHybridResult -s ${LUMI} --saveToys -T 5000 --setParameters lumiscale=${LUMI}



