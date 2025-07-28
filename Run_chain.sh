#!/bin/bash
#SBATCH --job-name=HZconfig
#SBATCH --qos=cu_hpc
#SBATCH --output=HZconfig.log
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=4G

source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh
cmsset

# This script try to run a chain of commands in MadGraph w/o cards
rm -rf HiggsStrahlungwithBDecay
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
generate e+ e- > z h, z > l+ l-, h > b b~
output HiggsStrahlungwithBDecay
launch -n formal01
shower=Pythia8

set run_card ebeam1 120
set run_card ebeam2 120
set run_card lpp1 0
set run_card lpp2 0
set run_card nevents 10000
EOF

rm -rf HiggsStrahlungwithQuarkDecay
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
generate e+ e- > z h, z > l+ l-, h > q q~
output HiggsStrahlungwithQuarkDecay
launch -n formal01
shower=Pythia8

set run_card ebeam1 120
set run_card ebeam2 120
set run_card lpp1 0
set run_card lpp2 0
set run_card nevents 10000
EOF

rm -rf HiggsStrahlungwithoutDecay
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
generate e+ e- > z h
output HiggsStrahlungwithoutDecay
launch -n formal01
shower=Pythia8

set run_card ebeam1 120
set run_card ebeam2 120
set run_card lpp1 0
set run_card lpp2 0
set run_card nevents 10000
EOF

rm -rf BDecay_output.root QuarkDecay_output.root NoDecay_output.root

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 BDecay_output.root HiggsStrahlungwithBDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 QuarkDecay_output.root HiggsStrahlungwithQuarkDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 NoDecay_output.root HiggsStrahlungwithoutDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz

 root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"BDecay_\",\"BDecay_output.root\")"
 root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"QuarkDecay_\",\"QuarkDecay_output.root\")"
 root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"NoDecay_\",\"NoDecay_output.root\")"