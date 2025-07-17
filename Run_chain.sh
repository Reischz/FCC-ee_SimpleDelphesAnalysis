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
