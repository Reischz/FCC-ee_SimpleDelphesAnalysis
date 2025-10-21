!
! Higgs MuTau decays
!Higgs:useBSM = on               ! use BSM
!HiggsH1:parity = 1
!25:m0 = 125.0                   ! higgs mass
!25:addChannel 1 0.1 100 15 -13  ! 1=decay mode on
!25:addChannel 1 0.1 100 13 -15
!25:onMode = off                 ! off other higgs decay
!25:onIfMatch 15 13              ! higgs will only LFV decay
!TauDecays:mode = 2              ! default decay mode
!TauDecays:tauPolarization = 0
!TauDecays:tauMother = 25        ! allow decay only tau from higgs tauMother
!PartonLevel:all = off           ! If off then stop the generation after the hard process has been generated
!
! Higgs EMu dacays
! NN:oneChannel = onMode bRatio meMode product1 product2 ...
!25:oneChannel 1 0.1 100 11 -13  ! 1=decay mode on,oneChannel decays channel before this is disable
!25:addChannel 1 0.1 100 13 -11  ! add another channel
!25:onIfAny 11 13                ! higgs will only decay either 11 or 13
!
!Main:numberOfEvents      = 10000
HEPMCoutput:file         = tag_1_pythia8_events.hepmc
SysCalc:fullCutVariation = off
Beams:frameType=4
HEPMCoutput:scaling=1.0000000000e+09
Check:epTolErr=1.0000000000e-02
JetMatching:etaJetMax=1.0000000000e+03
JetMatching:setMad=off
LHEFInputs:nSubruns=1
Main:subrun=0