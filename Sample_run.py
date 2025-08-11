# from lib import MadGraphPythiaDelphes_Chain as mgpd
from lib.MadGraphPythiaDelphes_Chain import GenerateSignalChains
import os
import yaml
from datetime import datetime

def RunConfig(process_config):
    mgpd = GenerateSignalChains(process_config)
    mgpd.begin_madgraph()
    mgpd.begin_extpythia()
    mgpd.begin_delphes()
    return 0

def yaml_runall(yamlContent):
    for item in yamlContent:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S") 
        dirName = f"{timestamp}_{item['Run_name']}_"
        print(f"Running process: {item['Run_name']}")
        os.makedirs(dirName, exist_ok=True)
        os.chdir(dirName)
        RunConfig(item)
        os.chdir('..')
    print("All processes have been run successfully.")
    return 0

def yaml_RunName(yamlContent, runName):
    for item in yamlContent:
        if runName in item['Run_name']:
            return item
        else:
            exit(1, f"Run name '{runName}' not found in YAML content.")

def main():
    yamlPath = os.path.join('lib', 'YamlConfig', 'FCC-ee_FourLeptonProcessesIncludeHiggsLFV.yaml')
    with open(yamlPath, 'r') as yamlFile:
        yamlContent = yaml.safe_load(yamlFile)
    print("YAML Content Loaded Successfully")
    #----------------------------------Run all processes----------------------------------
    yaml_runall(yamlContent)
    #----------------------------------Run specific process----------------------------------
    # process_config = yaml_RunName(yamlContent,'eeToZH_HtoWW_FourLeptons')
    # RunConfig(process_config)

if __name__ == "__main__":
    main()


