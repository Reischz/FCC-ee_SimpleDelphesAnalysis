#!/bin/bash

#SBATCH --qos=cu_htc
#SBATCH --partition=cpu
#SBATCH --job-name=ETauMuLFV
#SBATCH --output=sbatch.log
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=16G

# Clearing modules
#module purge

# Runing the bash script
./PYrun.sh
./DPrun.sh
