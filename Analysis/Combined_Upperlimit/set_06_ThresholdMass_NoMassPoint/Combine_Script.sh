#!/bin/bash
#SBATCH --job-name=combine
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpugpu
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
ZHTaTa_Yield=0.4157
ZHWW_Yield=0.5155
ZZTaTa_Yield=0.2719
ZWW4l_Yield=0.0056
LFV_Yield=(
    0.5770
    0.5755
    0.5725
    0.5704
    0.5688
    0.5663
    0.5632
    0.5598
)

Mass_Points=(110 115 120 125 130 135 140 145)
LUMI=1

COUNT=${#Mass_Points[@]}
# Expected Limit Band Calculations
for ((mass_index=0; mass_index<COUNT; mass_index++ )); do
    this_dir=Combine_Mass_${Mass_Points[mass_index]}
    rm -rf ${this_dir}
    mkdir -p ${this_dir}
    cd ${this_dir}

    # Prepare Data Card
    cp ../Data_Card_Combine.dat .
    sed -i "s/XXXX/${LFV_Yield[mass_index]}/g" Data_Card_Combine.dat
    sed -i "s/YYYY/${ZHTaTa_Yield}/g" Data_Card_Combine.dat
    sed -i "s/ZZZZ/${ZHWW_Yield}/g" Data_Card_Combine.dat
    sed -i "s/AAAA/${ZZTaTa_Yield}/g" Data_Card_Combine.dat
    sed -i "s/BBBB/${ZWW4l_Yield}/g" Data_Card_Combine.dat
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
    cd ..
done
wait
echo "All combine jobs Completed."

# Plotting the Test Statistic Distributions and Limits
# python3 /work/home/vwachira/CMSSW_14_1_0_pre5/src/HiggsAnalysis/CombinedLimit/test/plotTestStatCLs.py --input higgsCombineTest.HybridNew.mH120.${LUMI}.root --poi r --val all --mass 120

# Median Signal Strength Upper Limit Calculation without Systematics
# combine -M HybridNew --LHCmode LHC-limits no-bkg-test-nosyst.txt -n _nosyst --verbose 1 --saveHybridResult -s ${LUMI} --saveToys -T 5000 --setParameters lumiscale=${LUMI}



