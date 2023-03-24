from pyubx2 import UBXReader
from tqdm import tqdm
import sys, os

if len(sys.argv) <= 2:
    print(f"Usage: {sys.argv[0]} (outdir) (infile1) [infile2] [infile3] ...")
    sys.exit(1)

outdir = sys.argv[1]
infiles = sys.argv[2]

if not os.path.isdir(outdir):
    print("Output directory does not exist or is not a directory")
    sys.exit(1)


s = set()
with open(sys.argv[1], "rb") as f:
    header = f.read(3)

    ubr = UBXReader(f, protfilter=2)

    accum = 0
    for (raw_data, parsed_data) in ubr:
        accum += len(raw_data)

        if isinstance(parsed_data, str):
            print("UNKNOWN", repr(parsed_data))
            continue

        ident = parsed_data.identity
        if ident == "NAV-PVT":
            print(parsed_data)
            print(ident, len(raw_data), accum, parsed_data.hour*60*60 + parsed_data.min*60 +
                  parsed_data.second + parsed_data.nano*1e-9)
        else:
            print(ident, len(raw_data), accum)
