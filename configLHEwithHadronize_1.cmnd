! Higgs decays
Higgs:useBSM = on
HiggsH1:parity = 1
25:m0 = 200.0
25:addChannel 1 0.1 100 15 -13
25:addChannel 1 0.1 100 13 -15
25:onMode = off
25:onIfMatch 15 13
TauDecays:mode = 2
TauDecays:tauPolarization = 0
TauDecays:tauMother = 25
PartonLevel:all = off

! 1) Settings used in the main program.

Main:numberOfEvents = 100000         ! number of events to generate
Main:timesAllowErrors = 3          ! how many aborts before run stops

! 2) Settings related to output in init(), next() and stat().

Init:showChangedSettings = on      ! list changed settings
Init:showChangedParticleData = off ! list changed particle data
Next:numberCount = 100             ! print message every n events
Next:numberShowInfo = 1            ! print event information n times
Next:numberShowProcess = 1         ! print process record n times
Next:numberShowEvent = 1           ! print event record n times

! 3) Set the input LHE file

Beams:frameType = 4
Beams:LHEF = LFV_mutau/M200/pwgevents.lhe

! 4) Switch on/off the key event generation steps.
PartonLevel:all = on
HadronLevel:all = on

! Hadronization
Tune:pp 14
Tune:ee 7
MultipartonInteractions:ecmPow=0.03344
MultipartonInteractions:bProfile=2
MultipartonInteractions:pT0Ref=1.41
MultipartonInteractions:coreRadius=0.7634
MultipartonInteractions:coreFraction=0.63
ColourReconnection:range=5.176
SigmaTotal:zeroAXB=off
SpaceShower:alphaSorder=2
SpaceShower:alphaSvalue=0.118
SigmaProcess:alphaSvalue=0.118
SigmaProcess:alphaSorder=2
MultipartonInteractions:alphaSvalue=0.118
MultipartonInteractions:alphaSorder=2
TimeShower:alphaSorder=2
TimeShower:alphaSvalue=0.118
SigmaTotal:mode = 0
SigmaTotal:sigmaEl = 21.89
SigmaTotal:sigmaTot = 100.309
PDF:pSet=LHAPDF6:NNPDF31_nnlo_as_0118

! Random Seed
Random:setSeed = on
Random:seed = 1

! Additional setting blocks
!Check:epTolErr = 0.01 ! Recheck
Beams:setProductionScalesFromLHEF = off
!SLHA:minMassSM = 1000.
ParticleDecays:limitTau0 = on
ParticleDecays:tau0Max = 10
ParticleDecays:allowPhotonRadiation = on
