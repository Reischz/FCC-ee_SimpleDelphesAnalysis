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
# 150 GeV	0.530067	0.007599	0.122618	0.258248	0.000188
# 155 GeV	0.515198	0.004073	0.059976	0.192432	0.000043
# 160 GeV	0.509726	0.001489	0.024226	0.131328	0.000005
ZHTaTa_Yield=(
    0.007599
    0.004073
    0.001489
)
ZHWW_Yield=(
    0.122618
    0.059976
    0.024226
)
ZZTaTa_Yield=(
    0.258248
    0.192432
    0.131328
)

ZWW4l_Yield=(
    0.000188
    0.000043
    0.000005
)
LFV_Yield=(
    0.530067
    0.515198
    0.509726
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



