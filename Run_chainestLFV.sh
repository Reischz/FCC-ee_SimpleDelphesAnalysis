#!/bin/bash

# This script try to run a chain of commands in MadGraph w/o cards
rm -rf HiggsStrahlungwithDecay
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
generate e+ e- > w+ w-, w+ > l+ vl, w- > l- v~l, w+ > h b b~, h > b b~
output HiggsStrahlungwithDecay
launch -n formal01
shower=Pythia8

set run_card ebeam1 120
set run_card ebeam2 120
set run_card lpp1 0
set run_card lpp2 0
set run_card nevents 10000
EOF

source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh

rm -rf output01.root
gzip -dc HiggsStrahlungwithDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz \
 > HiggsStrahlungwithDecay/Events/formal01/tag_1_pythia8_events.hepmc

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 output01.root HiggsStrahlungwithDecay/Events/formal01/tag_1_pythia8_events.hepmc

