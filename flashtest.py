# Programs ESL Blaster remote mode data
# 2019 furrtek - furrtek.org
# See LICENSE

import pr
import tx
import sys
import serial

# Search for connected ESL Blaster
blaster_port = tx.search_esl_blaster()
if (blaster_port == "0"):
    exit()

ser = serial.Serial(blaster_port, 57600, timeout = 5)    # 5s timeout for read
ser.reset_input_buffer()

frameA = [0x00, 0x00, 0x20, 0x00, 0x10, 0x00, 0x0B, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0xAB, 0x09, 0x00, 0x00, 0xF2, 0xA7] # Segment change page
frameB = [0x00, 0x00, 0x20, 0x00, 0x10, 0x00, 0x0B, 0x00, 0x85, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x00, 0x00, 0xEA, 0x80] # DM change page

data = []
data.extend([0] * (1024 - len(data)))

data[0] = 2     # Frame count
data[1] = 0
data[2:2+len(frameA)] = frameA
data[2+72:2+72+len(frameB)] = frameB

print(len(data))

ba = bytearray()
ba.append('W')
ser.write(ba)
ser.flush()
for p in range(0, 8):
    ba = bytearray()
    for b in range(0, 128):
        ba.append(data[p*128 + b])

    ser.write(ba)
    ser.flush()
    ser.read_until('K')
    print("OK")

print(ser.read())

ser.close()

print("Done. Read flash to verify now.")
