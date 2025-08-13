import subprocess
import os

class GenerateSignalChains:
    def __init__(self,process_config):
        # self.MadGraphSwitch = True
        self.PythiaExternal = process_config['Pythia']['PythiaSwitch']
        # self.DelphesSwitch = True
        self.MadGraphPath = '/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC'
        self.PythiaInterfacePath = '/work/app/pythia8/MGInterface/1.3/MG5aMC_PY8_interface'
        self.DelphesPath = '/work/app/delphes/src/Delphes-3.5.0/DelphesHepMC2'
        self.MadGraphGen = process_config['madgraph']['gen_card']
        self.MadGraphRun = process_config['madgraph']['run_card']
        self.Run_name = process_config['Run_name']
        self.PythiaCard = process_config['Pythia']['PythiaCard']
        self.DelphesCardPath = process_config['DelphesCardPath']


    def begin_madgraph(self):
        gen_card_path = f'{self.Run_name}MadGraph_gen_card.dat'
        launch_card_path = f'{self.Run_name}MadGraph_launch_card.dat'
        with open(gen_card_path, 'w') as gen_card_file:
            gen_card_file.write(self.MadGraphGen)
        with open(launch_card_path, 'w') as launch_card_file:
            launch_card_file.write(self.MadGraphRun)
        run_DefinedEvent=subprocess.run([self.MadGraphPath, gen_card_path], capture_output=True, text=True, check=True)
        run_LaunchEvent=subprocess.run([self.MadGraphPath, launch_card_path], capture_output=True, text=True, check=True)
        with open(f'{self.Run_name}_MadGraph_output.log', 'w') as output_file:
            output_file.write(run_DefinedEvent.stdout + '\n' + run_LaunchEvent.stdout)
        Lhe_path = f'{self.Run_name}/Events/formal01/unweighted_events.lhe'
        if os.path.exists(Lhe_path):
            subprocess.run('mv ' + Lhe_path + ' ./' + self.Run_name + '_unweighted_events.lhe', shell=True)
            print(f"LHE file created successfully at {Lhe_path}")
        elif os.path.exists(f'{self.Run_name}/Events/formal01/unweighted_events.lhe.gz'):
            subprocess.run('gunzip' + f'{self.Run_name}/Events/formal01/unweighted_events.lhe.gz', shell=True)
            subprocess.run('mv ' + Lhe_path + ' ./' + self.Run_name + '_unweighted_events.lhe', shell=True)
            print(f"LHE file created successfully at {Lhe_path}")
        else:
            exit(1, "LHE file not created. Please check the MadGraph output.")
        return 0

    def begin_extpythia(self):
        pythia_events_path = f'{self.Run_name}/Events/formal01/tag_1_pythia8_events.hepmc'

        # Check permissions for the PythiaExt
        if not self.PythiaExternal:
            print("Pythia is not set to external. Skipping Pythia step.")
            hepmc_path = f'{self.Run_name}/Events/formal01/tag_1_pythia8_events.hepmc'
            if os.path.exists(hepmc_path):
                subprocess.run('mv ' + hepmc_path + ' ./' + self.Run_name + '_pythia8_events.hepmc', shell=True)
                print(f"Pythia events already exist for {self.Run_name}.")
            elif os.path.exists(hepmc_path + '.gz'):
                subprocess.run('gunzip' + hepmc_path + '.gz', shell=True)
                subprocess.run('mv ' + hepmc_path + ' ./' + self.Run_name + '_pythia8_events.hepmc', shell=True)
                print(f"Pythia events already exist for {self.Run_name}.")
            return 0
        # Check if the LHE file exists in current directory
        if not os.path.exists(f'{self.Run_name}_unweighted_events.lhe'):
            print("LHE file not found in current directory. Try locating it...")
        # Check Events directory for the LHE file
        elif os.path.exists(pythia_events_path):
            print(f"Pythia events already exist for {self.Run_name}.")
            subprocess.run('mv ' + pythia_events_path + ' ./' + self.Run_name + '_pythia8_events.hepmc', shell=True)
        # if not found, Go run madgraph
        else:
            print("LHE file not found. Please check the MadGraph output.")
            return 1
        
        # Set up Pythia card
        self.PythiaCard+= f'Beams:LHEF={self.Run_name}_unweighted_events.lhe\nHEPMC:output={self.Run_name}_pythia8_events.hepmc'
        with open(f'{self.Run_name}_pythia8_card.cmd', 'w') as pythia_card_file:
            pythia_card_file.write(self.PythiaCard)
        run_pythia = subprocess.run(
            [self.PythiaInterfacePath, f'{self.Run_name}_pythia8_card.cmd']
            , capture_output=True, text=True, check=True
        )
        with open(f'{self.Run_name}_pythia8_output.log', 'w') as output_file:
            output_file.write(run_pythia.stdout)
        return 0

    def begin_delphes(self):
        run_delphes = subprocess.run(
            [self.DelphesPath, self.DelphesCardPath, f'{self.Run_name}_pythia8_events.hepmc', f'{self.Run_name}_DelphesOutput.root'],
            capture_output=True, text=True, check=True
        )
        with open(f'{self.Run_name}_DelphesOutput.log', 'w') as output_file:
            output_file.write(run_delphes.stdout)
        return 0

# Test class
class HelloWorld:
    def __init__(self):
        self.check = "I'm a test variable"
    def greet(self):
        return "Hello, World!"