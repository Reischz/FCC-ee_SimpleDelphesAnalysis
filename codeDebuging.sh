#!/bin/bash
#SBATCH --job-name=test
#SBATCH --output=debuggingOutput.log
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpugpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=4G

# initiate HEP software environment
source /work/app/share_env/hepsw-gcc11p2-py3p9p9.sh
# >>> conda initialize >>>
# !! Contents within this block are managed by 'conda init' !!
__conda_setup="$('/work/home/ruttho/binary/miniconda3/bin/conda' 'shell.bash' 'hook' 2> /dev/null)"
if [ $? -eq 0 ]; then
    echo "DEBUG: Using conda hook method"
    eval "$__conda_setup"
else
    if [ -f "/work/home/ruttho/binary/miniconda3/etc/profile.d/conda.sh" ]; then
        echo "DEBUG: Using conda.sh profile script"
        . "/work/home/ruttho/binary/miniconda3/etc/profile.d/conda.sh"
    else
        echo "DEBUG: Using PATH fallback method"
        export PATH="/work/home/ruttho/binary/miniconda3/bin:$PATH"
    fi
fi
unset __conda_setup
# <<< conda initialize <<<

# Force conda environment Python to take precedence by moving it to front of PATH
export PATH="/work/home/ruttho/binary/miniconda3/envs/ext/bin:$PATH"  # NEEDED - conda activate doesn't work in HEP env!

# /work/home/ruttho/binary/miniconda3/envs/ext/bin/python pythontest.py
echo "Tell me which python is being used"
which python
echo "Conda environment python:"
/work/home/ruttho/binary/miniconda3/envs/ext/bin/python --version
echo "Python path:"
echo $PATH
