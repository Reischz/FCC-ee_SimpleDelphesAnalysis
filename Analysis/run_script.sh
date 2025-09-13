#!/bin/bash
#SBATCH --job-name=HZ4l
#SBATCH --qos=cu_hpc
#SBATCH --partition=cpu
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem=8G

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

conda activate HEP
python debug_python_analyze.py