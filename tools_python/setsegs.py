# Allows setting ESL segments
# 2019 furrtek - furrtek.org
# See LICENSE

import pr
import tx
import sys

def usage():
    print("Usage:")
    print("setsegs.py port barcode bitmap\n")
    print("  port: serial port name (0 for ESL Blaster)")
    print("  barcode: 17-character barcode data, or 0")
    print("  bitmap: hex 23-byte segment on/off bitmap")
    exit()

if len(sys.argv) != 4:
    usage()
if len(sys.argv[3]) != 46:
    print("Segment bitmap must be exactly 46 digits")
    usage()

port = sys.argv[1]

# Search for connected ESL Blaster if required
if (port == "0"):
    blaster_port = tx.search_esl_blaster()
    if (blaster_port == "0"):
        exit()

# Get PLID from barcode string
PLID = pr.get_plid(sys.argv[2])

bitmap = bytearray.fromhex(sys.argv[3])

frames = []

# Update page command + data
payload = [0xBA, 0x00, 0x00, 0x00]
payload.extend(bitmap)
# Segment bitmap has its own CRC16
segcrc = pr.crc16(bitmap)
payload.append(segcrc & 255)
payload.append((segcrc // 256) & 255)
# Page number, duration and some other unknown stuff
payload.extend([0x00, 0x00, 0x09, 0x00, 0x10, 0x00, 0x31])

frame = pr.make_raw_frame(0x84, PLID, payload[0])
frame.extend(payload[1:])
pr.terminate_frame(frame, 100)
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
