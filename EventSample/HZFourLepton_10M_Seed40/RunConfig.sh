#!/bin/bash
#SBATCH --job-name=HZ4l
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem=8G

source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh
echo 'Initailize MadGraphWPythia'
/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC HZFourLepton_10M_Seed40_mg5Card.dat
# try uzip file

gunzip eeToZH_HtoWW_FourLeptons/Events/formal01/tag_1_pythia8_events.hepmc.gz
mv eeToZH_HtoWW_FourLeptons/Events/formal01/unweighted_events.lhe.gz HZFourLepton_10M_Seed40_unweighted_events.lhe.gz
mv eeToZH_HtoWW_FourLeptons/Events/formal01/tag_1_pythia8_events.hepmc HZFourLepton_10M_Seed40_pythia8_events.hepmc
cp ../../delphes_card_IDEA.tcl .
echo 'set RandomSeed 40' >> delphes_card_IDEA.tcl
/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 delphes_card_IDEA.tcl \
 HZFourLepton_10M_Seed40.root HZFourLepton_10M_Seed40_pythia8_events.hepmc
echo 'Delphes done'
