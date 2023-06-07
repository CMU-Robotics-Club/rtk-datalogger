import socket
from pyrtcm import RTCMReader
import serial
import sys

stream = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
stream.connect((sys.argv[1], 5016))
rtr = RTCMReader(stream)

last = {}

for (raw_data, pd) in rtr.iterate():
    print(pd.identity)
    print(pd.GNSSEpoch)
