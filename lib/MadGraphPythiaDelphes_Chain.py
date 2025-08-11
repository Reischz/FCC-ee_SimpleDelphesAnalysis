import subprocess
import os

class GenerateSignalChains:
    def __init__(self,process_config):
        self.MadGraphSwitch = True
        self.PythiaExternal = True
        self.DelphesSwitch = True
        self.MadGraphPath = '/work/home/ruttho/binary/MG5_aMC_v2_9_24/bin/mg5_aMC'
        self.PythiaInterfacePath = '/work/home/ruttho/binary/pythia8-8305/bin/pythia8'
        self.DelphesPath = '/work/home/ruttho/binary/Delphes-3'
        self.MadGraphGen = process_config['MadGraph']['gen_card']
        self.MadGraphRun = process_config['MadGraph']['run_card']
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
            subprocess.run('ln -s ' + Lhe_path + ' ./' + self.Run_name + '_unweighted_events.lhe', shell=True)
            print(f"LHE file created successfully at {Lhe_path}")
        elif os.path.exists(f'{self.Run_name}/Events/formal01/unweighted_events.lhe.gz'):
            subprocess.run('gunzip -c ' + f'{self.Run_name}/Events/formal01/unweighted_events.lhe.gz', shell=True)
            subprocess.run('ln -s ' + Lhe_path + ' ./' + self.Run_name + '_unweighted_events.lhe', shell=True)
            print(f"LHE file created successfully at {Lhe_path}")
        else:
            exit(1, "LHE file not created. Please check the MadGraph output.")
        return 0

    def begin_extpythia(self):
        pythia_events_path = f'{self.Run_name}/Events/formal01/tag_1_pythia8_events.hepmc'
        if not os.path.exists(f'{self.Run_name}_unweighted_events.lhe'):
            exit(1, "LHE file not found. Please run MadGraph first.")
        elif os.path.exists(pythia_events_path):
            print(f"Pythia events already exist for {self.Run_name}.")
            subprocess.run('ln -s ' + pythia_events_path + ' ./' + self.Run_name + '_pythia8_events.hepmc', shell=True)
            return 0
        self.PythiaCard+= f'\nBeams:LHEF={self.Run_name}_unweighted_events.lhe\nHEPMC:output={self.Run_name}_pythia8_events.hepmc'
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