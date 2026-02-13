
! Force LFV H -> e +  mu

! 1. Clear all standard model decay channels for the Higgs (ID 25)
25:onMode = off
25:m0 = $MASS

! 2. Add the LFV decay channels for the Higgs
! H -> e+ mu-
25:oneChannel 1 0.5 100 11 -13
! H -> e- mu+
25:addChannel 1 0.5 100 -11 13

!
HEPMCoutput:file         = HEMu_LFV.hepmc
SysCalc:fullCutVariation = off
Beams:frameType=4
HEPMCoutput:scaling=1.0000000000e+09
Check:epTolErr=1.0000000000e-02
JetMatching:etaJetMax=1.0000000000e+03
JetMatching:setMad=off
LHEFInputs:nSubruns=1
Main:subrun=0
Main:numberOfEvents      = -1
Random:setSeed = on                ! Turn on the seed setting
Random:seed = 20
Beams:LHEF=$INFILE
