#!/bin/bash
#SBATCH --job-name=combine
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

# Event Yields Around Mass Point at 1 ab⁻¹ 240 GeV
#  	Signal	ZHTaTa	ZHWW	ZZTaTa	ZWW4l
# MassPoint	 	 	 	 	 
# 150 GeV	0.538041	0.053743	0.863733	0.791920	0.007519
# 155 GeV	0.522873	0.035062	0.582277	0.621072	0.003888
# 160 GeV	0.516834	0.022491	0.373341	0.485184	0.001787


ZHTaTa_Yield=(
    0.007599
    0.004073
    0.001489
)
ZHWW_Yield=(
    0.863633
    0.582277
    0.373341
)
ZZTaTa_Yield=(
    0.791920
    0.621072
    0.485184
)

ZWW4l_Yield=(
    0.007519
    0.003888
    0.001787
)
LFV_Yield=(
    0.538041
    0.522873
    0.516834
)

Mass_Points=(150 155 160)
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
    sed -i "s/YYYY/${ZHTaTa_Yield[mass_index]}/g" Data_Card_Combine.dat
    sed -i "s/ZZZZ/${ZHWW_Yield[mass_index]}/g" Data_Card_Combine.dat
    sed -i "s/AAAA/${ZZTaTa_Yield[mass_index]}/g" Data_Card_Combine.dat
    sed -i "s/BBBB/${ZWW4l_Yield[mass_index]}/g" Data_Card_Combine.dat
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



