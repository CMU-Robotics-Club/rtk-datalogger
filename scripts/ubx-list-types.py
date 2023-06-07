from pyubx2 import UBXReader
import sys

s = set()
with open(sys.argv[1], "rb") as f:
    ubr = UBXReader(f, protfilter=2)
    for (raw_data, parsed_data) in ubr:
        ident = parsed_data.identity
        if ident not in s:
            s.add(ident)
            print(ident)
