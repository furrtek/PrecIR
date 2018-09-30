# Allows transmitting raw frames to ESLs
# 2018 furrtek - furrtek.org
# See LICENSE

import pr
import sys

def usage():
    print("Usage:")
    print("rawcmd.py port barcode hex count")
    print("  port: serial port")
    print("  barcode: 17-character barcode data, or 0")
    print("  hex: frame data as hex string without CRC")
    print("  count: number of times the frame is transmitted")
    exit()

if len(sys.argv) != 5:
    usage()

# Get PLID from barcode string
PLID = pr.get_plid(sys.argv[2])

ba = bytearray.fromhex(sys.argv[3])

frames = []

frame = pr.make_raw_frame(PLID, ba[0])
frame.extend(ba[1:])
pr.terminate_frame(frame, int(sys.argv[4]))
frames.append(frame)

# DEBUG
#f = open("out.txt", "w")
#for fr in frames:
#    for b in fr:
#        f.write(format(b, '02X') + " ")
#    f.write("\n")
#exit()

# Send data to IR transmitter
pr.transmit_frames(frames, sys.argv[1])
print("Done.")
