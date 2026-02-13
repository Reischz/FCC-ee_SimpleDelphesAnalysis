#!/usr/bin/env python3
import os
import re
import sys
import gzip
import glob

def main():
    # Step 1: Find the .lhe.gz file (must be exactly one)
    lhe_files = glob.glob("*.lhe.gz")
    if len(lhe_files) != 1:
        print(f"Error: Expected exactly one .lhe.gz file, found {len(lhe_files)}.")
        sys.exit(1)

    lhe_file = lhe_files[0]

    # Step 2: Extract the mass number from the filename
    # Example: HZ4l_HLFV_1M_HMass110_Seed110_unweighted_events.lhe.gz
    match = re.search(r'HMass(\d+)', lhe_file)
    if not match:
        print("Error: Could not extract mass from file name.")
        sys.exit(1)

    mass = match.group(1)

    # Debug info
    print(f"Found LHE file: {lhe_file}")
    print(f"Extracted mass: {mass}")

    # Step 3: Update Pythia8Card_HEMu_LFV.cmd
    pythia_card = "Pythia8Card_HEMu_LFV.cmd"
    if not os.path.exists(pythia_card):
        print(f"Error: {pythia_card} not found.")
        sys.exit(1)

    with open(pythia_card, "r") as f:
        content = f.read()
    content = content.replace("$MASS", mass)
    content = content.replace("$INFILE", lhe_file)
    with open(pythia_card, "w") as f:
        f.write(content)
    print(f"Updated {pythia_card}")

    # Step 4: Update DPrun.sh
    dprun = "DPrun.sh"
    if not os.path.exists(dprun):
        print(f"Error: {dprun} not found.")
        sys.exit(1)

    with open(dprun, "r") as f:
        content = f.read()
    content = content.replace("$MASS", mass)
    with open(dprun, "w") as f:
        f.write(content)
    print(f"Updated {dprun}")

    print("\n✅ Done.")
    print(f"MASS = {mass}")
    print(f"LHE file = {lhe_file}")

if __name__ == "__main__":
    main()

