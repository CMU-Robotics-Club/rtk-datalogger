from pyubx2 import UBXReader
import sys

s = set()
with open(sys.argv[1], "rb") as f:
    ubr = UBXReader(f, protfilter=2)

    accum = 0
    while True:
        try:
            (raw_data, parsed_data) = next(ubr)
        except:
            #print("BAD")
            continue
        accum += len(raw_data)

        if isinstance(parsed_data, str):
            print("UNKNOWN", repr(parsed_data))
            continue

        ident = parsed_data.identity
        if ident == "NAV-PVT":
            print(parsed_data)
            print(ident, len(raw_data), accum, parsed_data.hour*60*60 + parsed_data.min*60 +
                  parsed_data.second + parsed_data.nano*1e-9)
        elif ident == "RXM-RAWX":
            #print(ident, parsed_data.rcvTow)
            pass
        else:
            #print(ident, len(raw_data), accum)
            pass
