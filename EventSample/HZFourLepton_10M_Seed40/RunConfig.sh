#!/bin/bash
#SBATCH --job-name=HZ4l10M
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem=20G

source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh
echo 'Initailize MadGraphWPythia'
# Create Gridpack
/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC HZFourLepton_1M_Seed10_mg5Card.dat
# Clean Directory
rm -rf eeToZH_HtoWW_FourLeptons
rm -rf *.lhe
rm -rf *.hepmc
rm -rf *.root
# Move grippack to working directory and unpack
mv eeToZH_HtoWW_FourLeptons/formal01_gridpack.tar.gz .
tar -xvf formal01_gridpack.tar.gz

# Begin Paralellel Event Generation using the gridpack
for i in {1..10}; do
    (
    # Event Generation
    echo "Starting Event Generation Job $i"
    mkdir -p job_$i
    cd job_$i
    SEED=$((10 + i))
    ../run.sh 1000000 $SEED
    echo "Event Generation Job $i done"

    # Set stage for Pythia8
    gunzip events.lhe.gz
    cp ../../../PythiaCard_Default.cmd .
    echo "Random:setSeed = on" >> PythiaCard_Default.cmd
    echo "Random:seed = $SEED" >> PythiaCard_Default.cmd
    echo "Beams:LHEF=events.lhe" >> PythiaCard_Default.cmd
    LD_LIBRARY_PATH=/work/app/pythia8/8.310/lib:$LD_LIBRARY_PATH\
     /work/app/pythia8/MGInterface/1.3/MG5aMC_PY8_interface PythiaCard_Default.cmd
    
    # Set stage for Delphes
    gunzip tag_1_pythia8_events.hepmc.gz
    cp ../../../delphes_card_IDEA.tcl .
    echo "set RandomSeed $SEED" >> delphes_card_IDEA.tcl
    /work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 delphes_card_IDEA.tcl \
     Delphes_job${i}.root tag_1_pythia8_events.hepmc

    # Move files back to main directory
    mv Delphes_job${i}.root ../HZFourLepton_1M_Part${i}_Seed${SEED}.root
    mv events.lhe.gz ../HZFourLepton_1M_Part${i}_Seed${SEED}_unweighted_events.lhe.gz
    mv tag_1_pythia8_events.hepmc ../HZFourLepton_1M_Part${i}_Seed${SEED}_pythia8_events.hepmc
    cd ..
    ) &
done
wait
echo "All Jobs Done"
