# Allows transmitting raw frames to ESLs
# 2018 furrtek - furrtek.org
# See LICENSE

import pr
import tx
import sys

def usage():
    print("Usage:")
    print("rawcmd.py port barcode type hex count\n")
    print("  port: serial port name (0 for ESL Blaster)")
    print("  barcode: 17-character barcode data, or 0")
    print("  type: DM for graphic ESL, SEG for segment")
    print("  hex: frame data as hex string without CRC")
    print("  count: number of times the frame is transmitted")
    exit()

if len(sys.argv) != 6:
    usage()

port = sys.argv[1]

# Search for connected ESL Blaster if required
if (port == "0"):
    blaster_port = tx.search_esl_blaster()
    if (blaster_port == "0"):
        exit()

# Get PLID from barcode string
PLID = pr.get_plid(sys.argv[2])

ba = bytearray.fromhex(sys.argv[4])

frames = []

frame = pr.make_raw_frame(0x85 if (sys.argv[3] == "DM") else 0x84, PLID, ba[0])
frame.extend(ba[1:])
pr.terminate_frame(frame, int(sys.argv[5]))
frames.append(frame)

# DEBUG
#f = open("out.txt", "w")
#for fr in frames:
#    for b in fr:
#        f.write(format(b, '02X') + " ")
#    f.write("\n")
#exit()

# Send data to IR transmitter
if (port == "0"):
    tx.transmit_esl_blaster(frames, blaster_port)
else:
    tx.transmit_serial(frames, port)
print("Done.")
