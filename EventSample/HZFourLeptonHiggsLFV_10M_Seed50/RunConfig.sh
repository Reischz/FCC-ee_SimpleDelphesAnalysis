#!/bin/bash
#SBATCH --job-name=HZHLFV4l
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem=8G

source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh
echo 'Initailize MadGraphWOPythia'
/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC HZFourLeptonHiggsLFV_10M_Seed50_mg5Card.dat
# try unzip file
mv eeToZH_HLFV_FourLeptons/Events/formal01/unweighted_events.lhe.gz .

# pythia8 events
echo 'Running Pythia8'
cp ../../PythiaCard_HiggsEMuLFV.cmd .
echo -e '\nMain:numberOfEvents      = 10000000' >> PythiaCard_HiggsEMuLFV.cmd
echo 'Random:setSeed = on                ! Turn on the seed setting' >> PythiaCard_HiggsEMuLFV.cmd
echo 'Random:seed = 50' >> PythiaCard_HiggsEMuLFV.cmd
echo 'Beams:LHEF=unweighted_events.lhe.gz' >> PythiaCard_HiggsEMuLFV.cmd
LD_LIBRARY_PATH=/work/home/ruttho/binary/MG5_aMC_v2_9_24/HEPTools/lib:$LD_LIBRARY_PATH\
 /work/home/ruttho/binary/MG5_aMC_v2_9_24/HEPTools/MG5aMC_PY8_interface/MG5aMC_PY8_interface PythiaCard_HiggsEMuLFV.cmd
gunzip tag_1_pythia8_events.hepmc.gz

mv unweighted_events.lhe.gz HZFourLeptonHiggsLFV_10M_Seed50_unweighted_events.lhe.gz
mv tag_1_pythia8_events.hepmc HZFourLeptonHiggsLFV_10M_Seed50_pythia8_events.hepmc
cp ../../delphes_card_IDEA.tcl .
echo 'set RandomSeed 50' >> delphes_card_IDEA.tcl
/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 delphes_card_IDEA.tcl \
 HZFourLeptonHiggsLFV_10M_Seed50.root HZFourLeptonHiggsLFV_10M_Seed50_pythia8_events.hepmc
echo 'Delphes done'
