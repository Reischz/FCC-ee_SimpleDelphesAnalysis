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

# Event Yields Around Mass Point
#  	Signal	ZHTaTa	ZHWW	ZZTaTa	ZWW4l
# MassPoint	 	 	 	 	 
# 110 GeV	0.576420	0.415706	0.515480	0.271928	0.005635
# 115 GeV	0.574319	0.281963	0.236219	0.119776	0.002901
# 120 GeV	0.570215	0.180237	0.101606	0.062928	0.001354
# 125 GeV	0.566646	0.109478	0.044766	0.035264	0.000654
# 130 GeV	0.563339	0.060072	0.019443	0.020976	0.000309
# 135 GeV	0.558806	0.029302	0.006821	0.011856	0.000124
# 140 GeV	0.552395	0.012943	0.002195	0.007600	0.000038
# 145 GeV	0.544145	0.004292	0.000784	0.004408	0.000008

ZHTaTa_Yield=(
    0.415706
    0.281963
    0.180237
    0.109478
    0.060072
    0.029302
    0.012943
    0.004292
)
ZHWW_Yield=(
    0.515480
    0.236219
    0.101606
    0.044766
    0.019443
    0.006821
    0.002195
    0.000784
)
ZZTaTa_Yield=(
    0.271928
    0.119776
    0.062928
    0.035264
    0.020976
    0.011856
    0.007600
    0.004408
)

ZWW4l_Yield=(
    0.005635
    0.002901
    0.001354
    0.000654
    0.000309
    0.000124
    0.000038
    0.000008
)
LFV_Yield=(
    0.576420
    0.574319
    0.570215
    0.566646
    0.563339
    0.558806
    0.552395
    0.544145
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



