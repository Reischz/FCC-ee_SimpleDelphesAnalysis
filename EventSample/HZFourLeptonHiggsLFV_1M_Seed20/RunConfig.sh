#!/bin/bash
#SBATCH --job-name=HZHLFV4l
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem=8G

source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh
echo 'Initailize MadGraphWOPythia'
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC HZFourLeptonHiggsLFV_1M_Seed20_mg5Card.dat
# try unzip file
mv eeToZH_HLFV_FourLeptons/Events/formal01/unweighted_events.lhe.gz .

# pythia8 events
echo 'Running Pythia8'
cp ../PythiaCard_HiggsEMuLFV.dat .
echo 'Main:numberOfEvents      = 1000' >> PythiaCard_HiggsEMuLFV.dat
echo 'Random:setSeed = on                ! Turn on the seed setting' >> PythiaCard_HiggsEMuLFV.dat
echo 'Random:seed = 20' >> PythiaCard_HiggsEMuLFV.dat
LD_LIBRARY_PATH=/work/home/ruttho/binary/MG5_aMC_v3_5_4/HEPTools/lib:$LD_LIBRARY_PATH\
 /work/home/ruttho/binary/MG5_aMC_v3_5_4/HEPTools/MG5aMC_PY8_interface/MG5aMC_PY8_interface PythiaCard_HiggsEMuLFV.dat
gunzip tag_1_pythia8_events.hepmc.gz

mv unweighted_events.lhe.gz HZFourLeptonHiggsLFV_1M_Seed20_unweighted_events.lhe.gz
mv tag_1_pythia8_events.hepmc HZFourLeptonHiggsLFV_1M_Seed20_pythia8_events.hepmc
cp ../delphes_card_IDEA.tcl .
echo 'set RandomSeed 20' >> delphes_card_IDEA.tcl
/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 delphes_card_IDEA.tcl \
 HZFourLeptonHiggsLFV_1M_Seed20.root HZFourLeptonHiggsLFV_1M_Seed20_pythia8_events.hepmc
echo 'Delphes done'
