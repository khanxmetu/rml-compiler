import argparse
import re
import subprocess
import shutil
from pathlib import Path

parser = argparse.ArgumentParser(description="Script to run and compare outputs of ic-samples")
parser.add_argument("--prog_path", type=Path, default="cmake-build-debug/driver")
parser.add_argument("--samples_in_dir", type=Path, default="ic-samples")
parser.add_argument("--samples_out_dir", type=Path, default="ic-samples-out")
args = parser.parse_args()

# Setup directories
args.samples_out_dir.mkdir(parents=True, exist_ok=True)

# Find all matching sample inputs using iterdir() and a cleaner regex
sample_pattern = re.compile(r"^sample\d+\.txt$")
samples = [p for p in args.samples_in_dir.iterdir() if sample_pattern.match(p.name)]

# Copy all .csv files from prog_path's parent to the out_dir
for csv_file in args.prog_path.parent.glob("*.csv"):
    shutil.copy(csv_file, args.samples_out_dir / csv_file.name)

# Process each sample
for sample in samples:
    # Copy sample to output directory
    shutil.copy(sample, args.samples_out_dir / sample.name)

    # .stem automatically gets the filename without the .txt extension
    sample_stem = sample.stem

    for opt in range(16):
        subprocess.run(
            [str(args.prog_path.absolute()), f"-p{opt}", sample_stem],
            cwd=args.samples_out_dir.absolute()
        )

        ic_file = args.samples_out_dir / f"{sample_stem}-IC.txt"
        if ic_file.exists():
            ic_file.rename(args.samples_out_dir / f"{sample_stem}-IC-p{opt:02}.txt")

        result_file = args.samples_out_dir / f"{sample_stem}result.txt"
        if result_file.exists():
            result_file.rename(args.samples_out_dir / f"{sample_stem}result-p{opt:02}.txt")

# Cleanup .csv files from the output directory
for csv_file in args.samples_out_dir.glob("*.csv"):
    csv_file.unlink()

# Diff the directories
subprocess.run([
    "diff",
    str(args.samples_in_dir),
    str(args.samples_out_dir),
    "-rqs"
])