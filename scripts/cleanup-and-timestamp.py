from pyubx2 import UBXReader
from tqdm import tqdm
import traceback
import sys, os, time

if len(sys.argv) <= 2:
    print(f"Usage: {sys.argv[0]} (outdir) (infile1) [infile2] [infile3] ...")
    sys.exit(1)

outdir = sys.argv[1]
infiles = sys.argv[2:]

if not os.path.isdir(outdir):
    print("Output directory does not exist or is not a directory")
    sys.exit(1)

files = []
totalsize = 0

for i, fname in enumerate(infiles):
    print(f"Loading file {i+1}/{len(infiles)} {fname}...")

    fsize = os.path.getsize(fname)
    with open(fname, "rb") as f:
        header = f.read(3)
        if header != b"GPS":
            print(f"File {fname} has invalid header")
            continue

    files.append(fname)
    totalsize += fsize

print("\n"*3)

with tqdm(total=totalsize) as pbar:
    for i, fname in enumerate(files):

        first_ts = None
        data_out = bytearray()

        with open(fname, "rb") as f:
            lastoffs = 0
            header = f.read(3)
            assert header == b"GPS"

            ubr = UBXReader(f, protfilter=2)

            accum = 0
            while True:
                try:
                    (raw_data, pd) = ubr.read()
                except:
                    print(f"Error at offset {f.tell()} in file {fname}")
                    traceback.print_exc(limit=1)
                    print()
                    print()
                    time.sleep(0.5)
                    continue

                if raw_data is None:
                    break

                pbar.update(f.tell() - lastoffs)
                lastoffs = f.tell()

                accum += len(raw_data)

                # Unknown protocol
                if isinstance(pd, str):
                    continue

                ident = pd.identity
                if ident == "NAV-PVT":
                    if pd.validDate and pd.validTime and first_ts is None:
                        first_ts = f"{pd.year:04d}_{pd.month:02d}_{pd.day:02d}_" + \
                                    f"{pd.hour:02d}_{pd.min:02d}_{pd.second:02d}"

                data_out += raw_data

            f.seek(0, 2)
            pbar.update(f.tell() - lastoffs)

        if first_ts is None:
            print(f"No valid PVT messages found in file {fname}")
        else:
            fn, _ = os.path.splitext(os.path.basename(fname))
            with open(os.path.join(outdir, first_ts+"_"+fn+".ubx"), "wb+") as f:
                f.write(data_out)
