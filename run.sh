#!/bin/bash
#SBATCH --job-name=test
#SBATCH --output=test.out
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem=8G

# >>> conda initialize >>>
# !! Contents within this block are managed by 'conda init' !!
__conda_setup="$('/work/home/ruttho/binary/miniconda3/bin/conda' 'shell.bash' 'hook' 2> /dev/null)"
if [ $? -eq 0 ]; then
    eval "$__conda_setup"
else
    if [ -f "/work/home/ruttho/binary/miniconda3/etc/profile.d/conda.sh" ]; then
        . "/work/home/ruttho/binary/miniconda3/etc/profile.d/conda.sh"
    else
        export PATH="/work/home/ruttho/binary/miniconda3/bin:$PATH"
    fi
fi
unset __conda_setup
# <<< conda initialize <<<

source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh
conda activate ext
python orchestratedFile.py