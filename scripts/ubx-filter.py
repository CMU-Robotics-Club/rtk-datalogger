from pyubx2 import UBXReader
import sys, os

infile = sys.argv[1]
outfile = sys.argv[2]
types = sys.argv[3:]

infile_size = os.stat(infile).st_size
parsed_size = 0
written_size = 0

with open(infile, "rb") as f:
    with open(outfile, "wb+") as f1:
        ubr = UBXReader(f, protfilter=2)
        for (raw_data, parsed_data) in ubr:
            parsed_size += len(raw_data)
            ident = parsed_data.identity

            if ident in types:
                written_size += len(raw_data)
                f1.write(raw_data)

            print(f"Read {parsed_size//1024}/{infile_size//1024} KB; Written {written_size//1024} KB")
