import socket
from pyrtcm import RTCMReader
import sys

stream = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
stream.connect((sys.argv[1], 5016))
rtr = RTCMReader(stream)

last = {}

for (raw_data, parsed_data) in rtr.iterate():
    #if parsed_data.identity == "1074":
    print(parsed_data.identity, len(raw_data))
    dd = vars(parsed_data)
    dd["_payload"] = None
    dd["_payload_bits"] = None

    dd1 = {}
    for k, v in dd.items():
        if last.get(k) != v:
            dd1[k] = (v, last.get(k))

    print(dd1)
    last = dd
