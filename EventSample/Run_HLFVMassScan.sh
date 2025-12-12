#!/bin/bash
#SBATCH --job-name=HZHLFV4l
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem=8G

echo 'Starting HZ Higgs LFV 4l Sample Mass Scan'
echo 'Initailize MadGraphWOPythia'
event_number=1000000
number_in_M=1M
# madgraph_path=/work/home/ruttho/binary/MG5_aMC_v2_9_24
# pythia_interface_path=/work/app/pythia8/MGInterface/1.3/MG5aMC_PY8_interface
# delphes_path=/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2
# make for loop for different masses
# mass points to scan 110 to 160 by 5
rm -rf MassScan_HZ4l_HLFV # remove old directory if exists
mkdir -p MassScan_HZ4l_HLFV
cd MassScan_HZ4l_HLFV
for mass in {110..160..5}
do
    target_dir=HZ4l_HLFV_${number_in_M}_HMass${mass}_Seed${mass}
    mkdir -p $target_dir
    cd $target_dir
    echo "Running mass point: $mass"

    # Run MadGraph5_aMC@NLO
    echo '-----------------------------------------------Running MadGraph5_aMC@NLO------------------------------------------------'
    mg5Card_name=HZ4l_HLFV_${number_in_M}_HMass${mass}_Seed${mass}_mg5Card.dat
    cp ../../Card/HZ4l_HLFV_mg5Card.dat $mg5Card_name
    sed -i "s/MASS_POINT/$mass/g" $mg5Card_name
    sed -i "s/EVENNUM_POINT/$event_number/g" $mg5Card_name
    sed -i "s/SEED_POINT/$mass/g" $mg5Card_name
    /work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC $mg5Card_name

    # Move lhe.gz file to current directory
    mv eeToZH_HLFV_FourLeptons/Events/formal01/unweighted_events.lhe.gz .
    # Make Pythia8 card
    pythiaCard_name=HZ4l_HLFV_PythiaCard_HMass${mass}_Seed${mass}.cmd
    cp ../../Card/HZ4lLFV/HZ4lLFV_PythiaCard.cmd $pythiaCard_name
    sed -i "s/MASS_POINT/$mass/g" $pythiaCard_name
    sed -i "s/SEED_POINT/$mass/g" $pythiaCard_name
    # Run Pythia8
    echo '-----------------------------------------------Running Pythia8------------------------------------------------'
    LD_LIBRARY_PATH=/work/app/pythia8/8.310/lib:$LD_LIBRARY_PATH\
     /work/app/pythia8/MGInterface/1.3/MG5aMC_PY8_interface $pythiaCard_name
    gunzip tag_1_pythia8_events.hepmc.gz

    # Configure Delphes
    cp ../../Card/delphes_card_IDEA.tcl .
    cp ../../Card/delphes_card_CEPC_RefTDR.tcl .
    echo 'set RandomSeed '"$mass"'' >> delphes_card_IDEA.tcl
    echo 'set RandomSeed '"$mass"'' >> delphes_card_CEPC_RefTDR.tcl
    echo '-----------------------------------------------Running Delphes------------------------------------------------'
    /work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 delphes_card_IDEA.tcl \
     HZ4l_HLFV_${number_in_M}_HMass${mass}_Seed${mass}_IDEA.root tag_1_pythia8_events.hepmc
    /work/home/ruttho/binary/delphes_cepc/DelphesHepMC2 delphes_card_CEPC_RefTDR.tcl \
     HZ4l_HLFV_${number_in_M}_HMass${mass}_Seed${mass}_CEPC_RefTDR_ChineseDelphes.root tag_1_pythia8_events.hepmc

    # Set output file names
    mv unweighted_events.lhe.gz HZ4l_HLFV_${number_in_M}_HMass${mass}_Seed${mass}_unweighted_events.lhe.gz
    mv tag_1_pythia8_events.hepmc HZ4l_HLFV_${number_in_M}_HMass${mass}_Seed${mass}_pythia8_events.hepmc
    echo "Completed mass point: $mass"
    cd ..
done