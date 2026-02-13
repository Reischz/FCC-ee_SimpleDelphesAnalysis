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

LUMI=1
DATABASE_FILE="yields_database.txt"

# Check if the C++ macro generated the database file
if [ ! -f "$DATABASE_FILE" ]; then
    echo "Error: $DATABASE_FILE not found! Run the C++ macro first."
    exit 1
fi

echo "Reading yields from $DATABASE_FILE..."

# Read the file line by line
# Format from C++: Mass Signal_Yield HZ4l_Yield ZWW4l_Yield
while read -r MASS LFV_YIELD HZ4L_YIELD ZWW4L_YIELD; do
    
    # Skip empty lines
    [ -z "$MASS" ] && continue
    
    echo "--------------------------------------------------------"
    echo "Processing Mass Point: ${MASS} GeV"
    echo "Signal: ${LFV_YIELD}, HZ4l: ${HZ4L_YIELD}, ZWW4l: ${ZWW4L_YIELD}"
    
    mkdir -p Combine_Mass_${MASS}
    cd Combine_Mass_${MASS}

    # Prepare Data Card
    cp ../Data_Card_Combine.dat .
    sed -i "s/XXXX/${LFV_YIELD}/g" Data_Card_Combine.dat
    sed -i "s/YYYY/${HZ4L_YIELD}/g" Data_Card_Combine.dat
    sed -i "s/ZZZZ/${ZWW4L_YIELD}/g" Data_Card_Combine.dat
    CARD="Data_Card_Combine_mh${MASS}.dat"
    mv Data_Card_Combine.dat ${CARD}

    # Create the workspace and run combine
    for quantile in -1 0.5 0.84 0.16 0.975 0.025; do
        if [ "$quantile" == "-1" ]; then
            echo "Calculating Observed Limit for ${MASS} GeV..."
            combine -M HybridNew --LHCmode LHC-limits ${CARD} --verbose 0 --saveHybridResult -s ${LUMI} \
                    --saveToys -T 5000 --setParameters lumiscale=${LUMI} --mass ${MASS} &
        else
            echo "Calculating Expected Limit (quantile ${quantile}) for ${MASS} GeV..."
            combine -M HybridNew --LHCmode LHC-limits ${CARD} --verbose 0 --saveHybridResult -s ${LUMI} \
                    --expectedFromGrid $quantile --saveToys -T 5000 --setParameters lumiscale=${LUMI} \
                    --mass ${MASS} &
        fi
    done
    
    # Wait for all background combine jobs for THIS mass point to finish before moving to the next
    wait 
    cd ..
    
done < "$DATABASE_FILE"

echo "All combine jobs Completed."