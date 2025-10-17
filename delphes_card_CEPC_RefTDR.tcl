####################################################################                                l
# CEPC Ref TDR model
#
# Maintainer: Kaili Zhang 
# email:  zhangkl@ihep.ac.cn
# 202409: First adaptation from CEPC 4th model.
# 202412: VTX move to stiching layout.
# 202508: Update to latest Ref-TDR Value
#####################################################################

## MOD2: set vtx mode timing to MC truth

set B 3.0

# Drift chamber coordinates
set DCHZMIN -2.98
set DCHZMAX  2.98
set DCHRMIN  0.6
set DCHRMAX  1.8
# PID algo need this geometry. While it only use the length in the DC/TPC.
#######################################
# Order of execution of various modules
#######################################

set ExecutionPath {
  
  TruthVertexFinder
  ParticlePropagator

  ChargedHadronTrackingEfficiency
  ElectronTrackingEfficiency
  MuonTrackingEfficiency

  TrackMergerPre
  TrackSmearing
  ClusterCounting
  TimeSmearing
  TimeOfFlight

  TrackMerger
  IdentificationMap
  Calorimeter

  TimeSmearingNeutrals
  TimeOfFlightNeutralHadron
  EFlowMerger
  PhotonFilter
  PhotonEfficiency
  PhotonIsolation


  ElectronFilter
  ElectronEfficiency
  ElectronIsolation

  MuonFilter
  MuonEfficiency

  MissingET

  NeutrinoFilter
  GenJetFinder
  GenMissingET

  FastJetFinder

  JetEnergyScale

  JetFlavorAssociation

  BTagging
  TauTagging

  TreeWriter
}

#################################
# Truth Vertex Finder
#################################

module TruthVertexFinder TruthVertexFinder {

  ## below this distance two vertices are assumed to be merged
  set Resolution 1E-06
  set InputArray Delphes/stableParticles
  set VertexOutputArray vertices
}

#################################
# Propagate particles in cylinder
#################################

module ParticlePropagator ParticlePropagator {
  set InputArray Delphes/stableParticles
  set OutputArray stableParticles
  set ChargedHadronOutputArray chargedHadrons
  set ElectronOutputArray electrons
  set MuonOutputArray muons

  # inner radius of the solenoid, in m <constant name="Solenoid_inner_radius" value="3535*mm"/>
  set Radius     3.535
  # half-length: z of the solenoid, in m <constant name="Solenoid_half_length" value="4575*mm"/>
  set HalfLength 4.575
  # magnetic field, in T
  set Bz         $B
}

####################################
# Charged hadron tracking efficiency
####################################

module Efficiency ChargedHadronTrackingEfficiency {
    set InputArray ParticlePropagator/chargedHadrons
    set OutputArray chargedHadrons
    set UseMomentumVector true

    # We use only one efficiency, we set only 0 effincency out of eta bounds:

    set EfficiencyFormula {
        (abs(eta) > 3.0)                               * (0.000) +
        (pt >= 0.5) * (abs(eta) <= 3.0)                * (0.997) +
        (pt <  0.5 && pt >= 0.1) * (abs(eta) <= 3.0)   * (0.950) +
        (pt <  0.1) * (abs(eta) <= 3.0)                * (0.060)
    }

}

##############################
# Electron tracking efficiency
##############################

module Efficiency ElectronTrackingEfficiency {
    set InputArray ParticlePropagator/electrons
    set OutputArray electrons
    set UseMomentumVector true

    # Current full simulation with CLICdet provides for electrons:
    set EfficiencyFormula {
        (abs(eta) > 3.0)                               * (0.000) +
        (pt >= 0.5) * (abs(eta) <= 3.0)                * (0.997) +
        (pt <  0.5 && pt >= 0.1) * (abs(eta) <= 3.0)   * (0.950) +
        (pt <  0.1) * (abs(eta) <= 3.0)                * (0.060)
    }

}


##########################
# Muon tracking efficiency
##########################

module Efficiency MuonTrackingEfficiency {
    set InputArray ParticlePropagator/muons
    set OutputArray muons
    set UseMomentumVector true

    # Current full simulation with CLICdet provides for muons:
    set EfficiencyFormula {
        (abs(eta) > 3.0)                               * (0.000) +
        (pt >= 0.5) * (abs(eta) <= 3.0)                * (0.997) +
        (pt < 0.5 && pt >= 0.1) * (abs(eta) <= 3.0)    * (0.950) +
        (pt < 0.1) * (abs(eta) <= 3.0)                 * (0.060)
    }
}
##############
# Track merger
##############

module Merger TrackMergerPre {
# add InputArray InputArray
  add InputArray  ChargedHadronTrackingEfficiency/chargedHadrons
  add InputArray  ElectronTrackingEfficiency/electrons
  add InputArray  MuonTrackingEfficiency/muons
  set OutputArray tracks
}


########################################
# Smearing for charged tracks
########################################

module TrackCovariance TrackSmearing {

    set InputArray  TrackMergerPre/tracks
    set OutputArray tracks

    ## minimum number of hits to accept a track
    set NMinHits 6

    ## magnetic field
    set Bz $B

    ## uses https://raw.githubusercontent.com/selvaggi/FastTrackCovariance/master/GeoIDEA_BASE.txt
    set DetectorGeometry {

      # Layer type 1 = R (barrel) or 2 = z (forward/backward)
      # Layer label
      # Minimum dimension z for barrel or R for forward
      # Maximum dimension z for barrel or R for forward
      # R/z location of layer
      # Thickness (meters)
      # Radiation length (meters)
      # Number of measurements in layers (1D or 2D)
      # Stereo angle (rad) - 0(pi/2) = axial(z) layer - Upper side
      # Stereo angle (rad) - 0(pi/2) = axial(z) layer - Lower side
      # Resolution Upper side (meters) - 0 = no measurement
      # Resolution Lower side (meters) - 0 = no measurement
      # measurement flag = T, scattering only = F

      # CEPC Ref-TDR geometry, Stitching layout. @202412.

      
      # barrel  name     zmin     zmax   r        w (m)      X0        n_meas  th_up (rad) th_down (rad)    reso_up (m)   reso_down (m)  flag
      # base geometry: https://code.ihep.ac.cn/cepc/CEPCSW/-/blob/master/Detector/DetCRD/compact/TDR_o1_v01/TDR_Dimensions_v01_01.xml
      # equivalent Length on r direction (to silicon) applied.
      # 140um silicon ~0.0015X0. 
      # Pipe inner r 10mm; 
      1	PIPE	-3.0	3.0	      0.010	    0.000151	0.0937	0	0	0	      0	        0	        0
      # Vertex geometry defined in: https://code.ihep.ac.cn/cepc/CEPCSW/-/blob/master/Detector/DetCRD/compact/CRD_common_v02/VXD_Composite_v01_02.xml
      1	VTX1A	-0.341	0.341	  0.011063	0.0000573	0.0937	2	0	1.5708	5e-006	  5e-006	  1
      1	VTX1B	-0.341	0.341	  0.016562	0.0000337	0.0937	2	0	1.5708	5e-006	  5e-006	  1
      1	VTX2A	-0.341  0.341	  0.022061	0.0000318	0.0937	2	0	1.5708	5e-006	  5e-006	  1
      1	VTX2B	-0.341  0.341	  0.027561	0.00004	  0.0937	2	0	1.5708	5e-006	  5e-006	  1
      1	VTX3A	-0.341  0.341	  0.039500	0.000316	0.0937	2	0	1.5708	5e-006	  5e-006	  1
      1	VTX3B	-0.341  0.341	  0.043240	0.000316	0.0937	2	0	1.5708	5e-006	  5e-006	  1 
      # Shell as Carbon fiber-reinforced plastic(CFRP). L=29.1cm.    0.000
      1	SHELL	-0.51	  0.51    0.0725	  0.0025	  0.291	  0	0	0	      0	        0	        0
      # vertex end at 72.5mm.
      # Silicon Inner Tracker (SIT for barrel and EST, endcap silicon tracker for Endcap).
      # Silicon tracker precision ~8-40um. 
      # 3 SIT layers 1.5% X0. ->1.4mm Si. Each 470mm.  
      # R: 8um ->Z: 40um.
      1	SIT01	-0.4933	0.4933	    0.235	  0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      1	SIT02	-0.7048	0.7048	    0.345	  0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      1	SIT03	-0.9866	0.9866	    0.5556	0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      # SIT end at r=600mm. z=500mm. 
      # TPC 1.2m length ~0.05X0. (0.25X0 for Z 0-2.9. ) X0: 364 for R direction; Or 728 for Z Direction.( split to r and z so 182)
      # For Z=0, r~60um. z~400um. Z=2.9, r~100um, z~1400um. 

      1  TPC_RP_VOLUME  -2.9  2.9   0.6 1.2 0.000182   1 0          0       0 60e-006  0             1
      1  TPC_Z_VOLUME   -2.9  2.9   0.6 1.2 0.000182   1 1.5708     1.5708  0 400e-006 0             1

      # single layer silicon, 2% X0. 1.8mm
      # EST 5 layers, each layer same as SIT.
      1	SET	   -2.93	 2.93	 1.8	 0.0018	 0.0937	  2	0	1.5708	8e-006	4e-005	 1
      #	endcap	name	rmin	rmax	z	w	(m)	X0	n_meas	th_up	(rad)	th_down	(rad)	reso_up	(m)	reso_down	(m)	flag
      2	EST1A	  0.075     0.24	 0.5055		    0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST1B	  0.075     0.24	-0.5055		    0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST2A	  0.1019  	0.35   0.720 		    0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST2B	  0.1019  	0.35  -0.720 	      0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST3A	  0.1426  	0.55   1.006 		    0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST3B	  0.1426    0.55	-1.006 		    0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST4A	  0.214   	0.55   1.500  	    0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST4B	  0.214   	0.55  -1.500  	    0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST5A	  0.35      1.800	 2.905        0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
      2	EST5B	  0.35      1.800	 2.905  	    0.00047	0.0937	2	0	1.5708	8e-006	4e-005	1
	}
	
}

##################
#VertexFinder
##################

# module VertexFinder VertexFinder {

#  add InputArray  TrackSmearing/tracks
#  set OutputArray tracks
#  set VertexOutputArray vertices
# }





###################
# Cluster Counting
###################

module ClusterCounting ClusterCounting {

  add InputArray TrackSmearing/tracks
  set OutputArray tracks

  set Bz $B

  ## check that these are consistent with DCHCANI/DCHNANO parameters in TrackCovariance module
  set Rmin $DCHRMIN
  set Rmax $DCHRMAX
  set Zmin $DCHZMIN
  set Zmax $DCHZMAX

  # gas mix option:
  # 0:  Helium 90% - Isobutane 10%
  # 1:  Helium 100%
  # 2:  Argon 50% - Ethane 50%
  # 3:  Argon 100%

  set GasOption 0
  # Current TPC value is extrapolated from old DC while 5% enhenced.
}


########################################
#   Time Smearing MIP
########################################
# tuning.
module TimeSmearing TimeSmearing {
  set InputArray ClusterCounting/tracks
  set OutputArray tracks

  # assume constant 30 ps resolution for now
  set TimeResolution {
                       (abs(eta) > 0.0 && abs(eta) <= 3.0)* 30E-12
                     }
}


########################################
#   Time Of Flight Measurement
########################################

module TimeOfFlight TimeOfFlight {
  set InputArray TimeSmearing/tracks
  set VertexInputArray TruthVertexFinder/vertices

  set OutputArray tracks

  # 0: assume vertex time tV from MC Truth (ideal case)
  # 1: assume vertex time tV = 0
  # 2: calculate vertex time as vertex TOF, assuming tPV=0
  # tV is the initial time for the track. and time smearing is to smear the end time.
  set VertexTimeMode 0
}

##############
# Track merger
##############

module Merger TrackMerger {
# add InputArray InputArray
  add InputArray  TimeOfFlight/tracks
  set OutputArray tracks
}


#############
# Calorimeter
#############
module DualReadoutCalorimeter Calorimeter {
  set ParticleInputArray ParticlePropagator/stableParticles
  set TrackInputArray    IdentificationMap/tracks

  set TowerOutputArray   towers
  set PhotonOutputArray  photons

  set EFlowTrackOutputArray         eflowTracks
  set EFlowPhotonOutputArray        eflowPhotons
  set EFlowNeutralHadronOutputArray eflowNeutralHadrons

  set ECalEnergyMin 0.5
  set HCalEnergyMin 0.5
  set EnergyMin     0.5
  set ECalEnergySignificanceMin 1.0
  set HCalEnergySignificanceMin 1.0
  set EnergySignificanceMin     1.0

  set SmearTowerCenter true
  #set SmearTowerCenter false
  set pi [expr {acos(-1)}]

    # Lists of the edges of each tower in eta and phi;
    # each list starts with the lower edge of the first tower;
    # the list ends with the higher edged of the last tower.
    # CEPC v4:
    # Barrel:  deta=0.01 towers up to |eta| <= 0.88 ( up to 45°)
    # Endcaps: deta=0.01 towers up to |eta| <= 3.0 (8.6° = 100 mrad)
    # CEPC refTDR:
    # Barrel:  deta=0.01 towers up to |eta| <= 1.24 ( up to 58°)
    # Endcaps: deta=0.01 towers up to |eta| <= 3.0 (8.6° = 100 mrad)
    # Cell size: about 6 cm x 6 cm

    #barrel:
    set PhiBins {}
    for {set i -120} {$i <= 120} {incr i} {
        add PhiBins [expr {$i * $pi/120}]
    }
    #deta=0.01 units for |eta| <= 1.24
    for {set i -62} {$i < 63} {incr i} {
        set eta [expr {$i * 0.02}]
        add EtaPhiBins $eta $PhiBins
    }

    #endcaps:
    set PhiBins {}
    for {set i -120} {$i <= 120} {incr i} {
        add PhiBins [expr {$i* $pi/120}]
    }
    #deta=0.01 units for 0.88 < |eta| <= 3.0
    #first, from -3.0 to -1.24
    for {set i 0} {$i <88} {incr i} {
        set eta [expr {-3.00 + $i * 0.02}]
        add EtaPhiBins $eta $PhiBins
    }
    #same for 1.24 to 3.0
    for  {set i 1} {$i <=88} {incr i} {
        set eta [expr {1.24 + $i * 0.02}]
        add EtaPhiBins $eta $PhiBins
    }

    # default energy fractions {abs(PDG code)} {Fecal Fhcal}
    add EnergyFraction {0} {0.0 1.0}
    # energy fractions for e, gamma and pi0
    add EnergyFraction {11} {1.0 0.0}
    add EnergyFraction {22} {1.0 0.0}
    add EnergyFraction {111} {1.0 0.0}
    # energy fractions for muon, neutrinos and neutralinos
    add EnergyFraction {12} {0.0 0.0}
    add EnergyFraction {13} {0.1 0.2}
    add EnergyFraction {14} {0.0 0.0}
    add EnergyFraction {16} {0.0 0.0}
    add EnergyFraction {1000022} {0.0 0.0}
    add EnergyFraction {1000023} {0.0 0.0}
    add EnergyFraction {1000025} {0.0 0.0}
    add EnergyFraction {1000035} {0.0 0.0}
    add EnergyFraction {1000045} {0.0 0.0}
    # energy fractions for K0short and Lambda
    add EnergyFraction {310} {0.3 0.7}
    add EnergyFraction {130} {0.3 0.7}
    add EnergyFraction {3122} {0.3 0.7}
    # energy fractions for pi
    add EnergyFraction {211} {0.3 0.7}
    add EnergyFraction {321} {0.3 0.7}
    add EnergyFraction {2212} {0.3 0.7}
    # set ECalResolutionFormula {resolution formula as a function of eta and energy}
    set ECalResolutionFormula {
    (abs(eta) <= 0.88 )                     * sqrt(energy^2*0.00456^2 + energy*0.001118^2)+
    (abs(eta) > 0.88 && abs(eta) <= 3.0)    * sqrt(energy^2*0.00456^2 + energy*0.001118^2)
    }

    # set HCalResolutionFormula {resolution formula as a function of eta and energy}
    set HCalResolutionFormula {
    (abs(eta) <= 0.88 )                     * sqrt(energy^2*0.05^2 + energy*0.35^2)+
    (abs(eta) > 0.88 && abs(eta) <= 3.0)    * sqrt(energy^2*0.05^2 + energy*0.35^2)
    }
}




####################################
# Charged hadron PID
####################################

module IdentificationMap IdentificationMap {
  set InputArray TrackMerger/tracks
  set OutputArray tracks

}
########################################
#   Time Smearing Neutrals
########################################

module TimeSmearing TimeSmearingNeutrals {
  set InputArray  Calorimeter/eflowNeutralHadrons
  set OutputArray eflowNeutralHadrons

  # assume constant 30 ps resolution for now
  set TimeResolution {
                       (abs(eta) > 0.0 && abs(eta) <= 3.0)* 30E-12
                     }
}

########################################
#   Time Of Flight Measurement
########################################

module TimeOfFlight TimeOfFlightNeutralHadron {
  set InputArray       TimeSmearingNeutrals/eflowNeutralHadrons
  set VertexInputArray TruthVertexFinder/vertices
  set OutputArray      eflowNeutralHadrons

  # 0: assume vertex time tV from MC Truth (ideal case)
  # 1: assume vertex time tV = 0
  # 2: calculate vertex time as vertex TOF, assuming tPV=0

  ## TBF (add option to take hard vertex time)
  set VertexTimeMode 1
}

####################
# Energy flow merger
####################

module Merger EFlowMerger {
  add InputArray  Calorimeter/eflowTracks
  add InputArray  Calorimeter/eflowPhotons
  add InputArray  TimeOfFlightNeutralHadron/eflowNeutralHadrons
  set OutputArray eflow
}


###################
# Photon efficiency
###################

module Efficiency PhotonEfficiency {
  set InputArray  Calorimeter/eflowPhotons
  set OutputArray photons

  # set EfficiencyFormula {efficiency formula as a function of eta and pt}
  # efficiency formula for photons
  set EfficiencyFormula {
        (energy <  2.0)                                       * (0.000) +
        (energy >= 2.0) * (abs(eta) <= 0.88)                  * (0.990) +
        (energy >= 2.0) * (abs(eta) >  0.88&&abs(eta) <= 3.0) * (0.990) +
        (abs(eta) > 3.0)                                      * (0.000)
  }
}

##################
# Photon isolation
##################

module Isolation PhotonIsolation {
  set CandidateInputArray PhotonEfficiency/photons
  set IsolationInputArray EFlowMerger/eflow

  set OutputArray photons
  set DeltaRMax 0.4
  set PTMin 0.5
  set PTRatioMax 0.12
}

#################
# Photon filter
#################

module PdgCodeFilter PhotonFilter {
  set InputArray EFlowMerger/eflow
  set OutputArray1 WoPhotonpair
  set OutputArray2 Photonpair

  add EnMin   {35.0}
  add MassRes {125.25}
  add NP      {2}

  add PdgCode {22}
}

#################
# Electron filter
#################

module PdgCodeFilter ElectronFilter {
  set InputArray Calorimeter/eflowTracks
  set OutputArray1 electrons
  set Invert true
  add PdgCode { 11}
  add PdgCode {-11}
}



#####################
# Electron efficiency
#####################

module Efficiency ElectronEfficiency {
  set InputArray ElectronFilter/electrons
  set OutputArray electrons

  # set EfficiencyFormula {efficiency formula as a function of eta and pt}
  # efficiency formula for electrons
  set EfficiencyFormula {
        (energy <  2.0)                                       * (0.000)+
        (energy >= 2.0) * (abs(eta) <= 0.88)                  * (0.990)+
        (energy >= 2.0) * (abs(eta) >0.88 && abs(eta) <= 3.0) * (0.990)+
        (abs(eta)> 3.0)                                       * (0.000)
  }
}

####################
# Electron isolation
####################

module Isolation ElectronIsolation {
  set CandidateInputArray ElectronEfficiency/electrons
  set IsolationInputArray EFlowMerger/eflow
  set OutputArray         electrons

  set DeltaRMax  0.50
  set PTMin      0.50
  set PTRatioMax 0.12
}

#################
# Muon filter
#################

module PdgCodeFilter MuonFilter {
  set InputArray EFlowMerger/eflow
  set OutputArray1 Muon
  set Invert true
  set PTMin      0.5
#  set OutputArray2 MuonPair
#  add EnMin   {15.0}
#  add MassRes {91.18}
#  add NP      {2}

  add PdgCode {13}
  add PdgCode {-13}
}

#################
# Muon efficiency
#################

module Efficiency MuonEfficiency {
  set InputArray  MuonFilter/Muon
  set OutputArray muons

  # set EfficiencyFormula {efficiency as a function of eta and pt}
  # efficiency formula for muons
  set EfficiencyFormula {
        (energy < 2.0)                                         * (0.000)+
        (energy >= 2.0) * (abs(eta) <= 0.88)                   * (0.990)+
        (energy >= 2.0) * (abs(eta) >0.88 && abs(eta) <= 3.0)  * (0.990)+
        (abs(eta) > 3.0)                                       * (0.000)
  }
}

###################
# Missing ET merger
###################

module Merger MissingET {
  add InputArray EFlowMerger/eflow
  set MomentumOutputArray momentum
}

##################
# Scalar HT merger
##################

module Merger ScalarHT {
  add InputArray        UniqueObjectFinder/jets
  add InputArray        UniqueObjectFinder/electrons
  add InputArray        UniqueObjectFinder/photons
  add InputArray        UniqueObjectFinder/muons
  set EnergyOutputArray energy
}

#####################
# Neutrino Filter
#####################

module PdgCodeFilter NeutrinoFilter {

  set InputArray  Delphes/stableParticles
  set OutputArray1 filteredParticles
  set PTMin 0.0

  add PdgCode { 12}
  add PdgCode { 14}
  add PdgCode { 16}
  add PdgCode {-12}
  add PdgCode {-14}
  add PdgCode {-16}
}


#####################
# MC truth jet finder
#####################

module FastJetFinder GenJetFinder {
  set InputArray NeutrinoFilter/filteredParticles
  set OutputArray jets
  set ExclusiveClustering true  

  set JetAlgorithm 11
  #set ParameterR   1.5
  set ParameterP   1.0
  #set JetPTMin     1.0
  set Njets       2

}

#########################
# Gen Missing ET merger
########################

module Merger GenMissingET {
  add InputArray NeutrinoFilter/filteredParticles
  set MomentumOutputArray momentum
}

############
# Jet finder
############

module FastJetFinder FastJetFinder {
  set InputArray EFlowMerger/eflow
  set OutputArray jets
  set ExclusiveClustering true 

  # modules/fastjetfinder.cc    10 as eegenkt algo.  11 for eekt algo.
  set JetAlgorithm 11
  #set ParameterR   1.5
  set ParameterP   1.0
  set Njets        2
  #set JetPTMin     0.0
}

##################
# Jet Energy Scale
##################

module EnergyScale JetEnergyScale {
  set InputArray FastJetFinder/jets
  set OutputArray jets

  # scale formula for jets
  set ScaleFormula {1.025}
}

########################
# Jet Flavor Association
########################

module JetFlavorAssociation JetFlavorAssociation {

  set PartonInputArray Delphes/partons
  set ParticleInputArray Delphes/allParticles
  set ParticleLHEFInputArray Delphes/allParticlesLHEF
  set JetInputArray JetEnergyScale/jets

  set DeltaR       0.5
  set PartonPTMin  1.0
  set PartonEtaMax 3.0
}

###########
# b-tagging
###########

module BTagging BTagging {
  set JetInputArray JetEnergyScale/jets
  set BitNumber 0

  # add EfficiencyFormula {abs(PDG code)} {efficiency formula as a function of eta and pt}

  # default efficiency formula (misidentification rate)
  add EfficiencyFormula {0} {0.001}

  # efficiency formula for c-jets (misidentification rate)
  add EfficiencyFormula {4} {0.01}

  # efficiency formula for b-jets
  add EfficiencyFormula {5} {0.95}
}

#############
# tau-tagging
#############

module TauTagging TauTagging {
  set ParticleInputArray Delphes/allParticles
  set PartonInputArray Delphes/partons
  set JetInputArray JetEnergyScale/jets

  set DeltaR    0.5
  set TauPTMin  1.0
  set TauEtaMax 3.0

  # default efficiency formula (misidentification rate)
  add EfficiencyFormula { 0} {0.001}
  # efficiency formula for tau-jets
  add EfficiencyFormula {15} {0.990}
}


##################
# ROOT tree writer
##################

# Tracks, towers and eflow objects are not stored by default in the output.
# If needed (for jet constituent or other studies), uncomment the relevant
# "add Branch ..." lines.

module TreeWriter TreeWriter {
    # add Branch InputArray BranchName BranchClass
    add Branch Delphes/allParticles Particle GenParticle
    # add Branch TruthVertexFinder/vertices GenVertex Vertex
    # add Branch VertexFinder/vertices Vertex Vertex

    # add Branch IdentificationMap/tracks  Track Track
    #add Branch Calorimeter/towers Tower Tower

    # add Branch Calorimeter/eflowTracks EFlowTrack Track
    #add Branch Calorimeter/eflowPhotons EFlowPhoton Tower
    #add Branch TimeOfFlightNeutralHadron/eflowNeutralHadrons EFlowNeutralHadron Tower

    add Branch  EFlowMerger/eflow ParticleFlowCandidate ParticleFlowCandidate

    # add Branch Calorimeter/photons CaloPhoton Photon
    # add Branch PhotonEfficiency/photons PhotonEff Photon
    # add Branch PhotonIsolation/photons PhotonIso Photon
    # add Branch PhotonFilter/Photonpair Photonpair Photon

    add Branch GenJetFinder/jets GenJet Jet
    add Branch GenMissingET/momentum GenMissingET MissingET

    add Branch JetEnergyScale/jets Jet Jet
    add Branch ElectronIsolation/electrons Electron Electron
    add Branch PhotonIsolation/photons Photon Photon
    add Branch MuonEfficiency/muons Muon Muon
    # add Branch MuonFilter/WoMuonPair WoMuonPair Muon

    add Branch MissingET/momentum MissingET MissingET

    # add Info InfoName InfoValue
    add Info Bz $B
}
