#!/bin/bash

# This script try to run a chain of commands in MadGraph w/o cards
rm -rf HiggsStrahlungwithDecay
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
generate e+ e- > z h, z > l+ l-, h > b b~
output HiggsStrahlungwithDecay
launch -n formal01
shower=Pythia8

set run_card ebeam1 120
set run_card ebeam2 120
set run_card lpp1 0
set run_card lpp2 0
set run_card nevents 10000
EOF

rm -rf output01.root

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 output01.root HiggsStrahlungwithDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz


#!/bin/bash

#export ROOT_INCLUDE_PATH=/work/app/delphes/src/Delphes-3.5.0/external
#export LD_LIBRARY_PATH=/work/app/pythia8/src/pythia8310/lib:$LD_LIBRARY_PATH
source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh

rm -rf output01.root
#/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC3 \
# "/work/home/ruttho/binary/extension/delphes_additionalCard/delphes_card_FCCeeDet$#  output01.root \
#  HiggsStrahlungwithDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz

gzip -dc HiggsStrahlungwithDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz \    > HiggsStrahlungwithDecay/Events/formal01/tag_1_pythia8_events.hepmc

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 output01.root HiggsStrahlungwithDecay/Events/formal01/tag_1_pythia8_events.hepmc
