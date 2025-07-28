#!/bin/bash
#SBATCH --job-name=HZconfig
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --output=HZconfig.log
#SBATCH --ntasks=3
#SBATCH --cpus-per-task=1
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

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 BDecay_output.root HiggsStrahlungwithBDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz

root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"BDecay_\",\"BDecay_output.root\")"
}

run_qdecay() {
rm -rf HiggsStrahlungwithQuarkDecay
rm -rf QuarkDecay_output.root
/work/home/ruttho/binary/MG5_aMC_v3_5_4/bin/mg5_aMC << EOF
define multiparticles q = u d s c b t
define particles q~ = u~ d~ s~ c~ b~ t~
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

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 QuarkDecay_output.root HiggsStrahlungwithQuarkDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz

root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"QuarkDecay_\",\"QuarkDecay_output.root\")"
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

/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2 \
 /work/app/delphes/src/Delphes-3.5.0/cards/delphes_card_IDEA.tcl \
 NoDecay_output.root HiggsStrahlungwithoutDecay/Events/formal01/tag_1_pythia8_events.hepmc.gz

root -l -b -q "root_analysisCode/plot_allFinalProcessInfo.cpp(\"NoDecay_\",\"NoDecay_output.root\")"
}

# Run all in parallel
run_bdecay &
run_qdecay &
run_nodecay &

# Wait for all to complete
wait

echo "All decay tasks completed."