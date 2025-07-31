#!/bin/bash
#SBATCH --job-name=ZWWFour
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem=8G

source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh

# This script try to run a chain of commands in MadGraph w/o cards
run_bdecay() {
rm -rf HiggsStrahlungwithBDecay
rm -rf BDecay_output.root
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

gzip -dc HiggsStrahlungwithBDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz \
 > HiggsStrahlungwithBDecay/Events/formal01/tag_1_pythia8_events.hepmc

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 BDecay_output.root HiggsStrahlungwithBDecay/Events/formal01/tag_1_pythia8_events.hepmc

root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"BDecay_\",\"BDecay_output.root\")"
root -l -b -q "root_analysisCode/plot_InvariantMass.cpp(\"BDecay_\",\"BDecay_output.root\")"
}

run_qdecay() { 
rm -rf HiggsStrahlungwithQuarkDecay
rm -rf QuarkDecay_output.root
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
define q = u d s c b t
define q~ = u~ d~ s~ c~ b~ t~
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

gzip -dc HiggsStrahlungwithQuarkDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz \
 > HiggsStrahlungwithQuarkDecay/Events/formal01/tag_1_pythia8_events.hepmc

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 QuarkDecay_output.root HiggsStrahlungwithQuarkDecay/Events/formal01/tag_1_pythia8_events.hepmc

root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"QuarkDecay_\",\"QuarkDecay_output.root\")"
root -l -b -q "root_analysisCode/plot_InvariantMass.cpp(\"QuarkDecay_\",\"QuarkDecay_output.root\")"
}

run_nodecay() {
rm -rf HiggsStrahlungwithoutDecay
rm -rf NoDecay_output.root
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

gzip -dc HiggsStrahlungwithoutDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz \
 > HiggsStrahlungwithoutDecay/Events/formal01/tag_1_pythia8_events.hepmc

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 NoDecay_output.root HiggsStrahlungwithoutDecay/Events/formal01/tag_1_pythia8_events.hepmc

root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"NoDecay_\",\"NoDecay_output.root\")"
root -l -b -q "root_analysisCode/plot_InvariantMass.cpp(\"NoDecay_\",\"NoDecay_output.root\")"
}

run_HZFourLeptons() {
local output_dir=${1:- "HiggsStrahlungwithFourLeptons"}
local process=${2:- "e+ e- > z h, (h > w+ w-, w+ > l+ vl, w- > l- vl~), z > l+ l-"}
rm -rf $output_dir
rm -rf $output_dir.root
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
generate $process
output $output_dir
launch -n formal01
shower=Pythia8

set nb_cores 10
set run_card ebeam1 120
set run_card ebeam2 120
set run_card lpp1 0
set run_card lpp2 0
set run_card nevents 10000
EOF
gzip -dc $output_dir/Events/formal01/tag_1_pythia8_events.hepmc.gz \
 > $output_dir/Events/formal01/tag_1_pythia8_events.hepmc
/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 $output_dir.root $output_dir/Events/formal01/tag_1_pythia8_events.hepmc
root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"${output_dir}_\",\"$output_dir.root\")"
root -l -b -q "root_analysisCode/plot_InvariantMass.cpp(\"${output_dir}_\",\"$output_dir.root\")"
}

run_sde_strategyMode() {
local output_dir=${1:- "HiggsStrahlungwithFourLeptons"}
local process=${2:- "e+ e- > z h, (h > w+ w-, w+ > l+ vl, w- > l- vl~), z > l+ l-"}
rm -rf $output_dir
rm -rf $output_dir.root
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
generate $process
output $output_dir
launch -n formal01
shower=Pythia8

set nb_cores 10
set run_card ebeam1 120
set run_card ebeam2 120
set run_card lpp1 0
set run_card lpp2 0
set run_card nevents 10000
set sde_strategy 2
EOF
gzip -dc $output_dir/Events/formal01/tag_1_pythia8_events.hepmc.gz \
 > $output_dir/Events/formal01/tag_1_pythia8_events.hepmc
/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 $output_dir.root $output_dir/Events/formal01/tag_1_pythia8_events.hepmc
root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"${output_dir}_\",\"$output_dir.root\")"
root -l -b -q "root_analysisCode/plot_InvariantMass.cpp(\"${output_dir}_\",\"$output_dir.root\")"
}


#run_HZFourLeptons "HiggsStrahlungwithFourLeptons" "e+ e- > z h, (h > w+ w-, w+ > l+ vl, w- > l- vl~), z > l+ l-"
run_sde_strategyMode "HWWFourLepton" "e+ e- > z w- w+, w- > l- vl~, w+ > l+ vl, z > l+ l-"

echo "All decay tasks completed."
