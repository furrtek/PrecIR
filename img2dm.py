# Converts, compresses and transmits images to dot matrix ESLs
# 2018 furrtek - furrtek.org
# See LICENSE

import pr
import tx
from imageio import imread
import sys

def record_run(run_count):
    # Convert to binary
    del bits[:]
    while run_count:
        bits.insert(0, run_count & 1)
        run_count >>= 1
    # Zero length coding
    for b in bits[1:]:
        compressed.append(0)
    if len(bits):
        compressed.extend(bits)

def usage():
    print("img2dm - Transmits image to Dot Matrix ESL\n")
    print("Usage:")
    print("img2dm.py port image barcode page (x y)\n")
    print("  port: serial port name (0 for ESL Blaster)")
    print("  image: image file")
    print("  barcode: 17-character barcode data")
    print("  page: page number to update (0~15)")
    print("  x y: top-left position of image, default: 0 0")
    exit()

arg_count = len(sys.argv)
if arg_count < 5:
    usage()

port = sys.argv[1]

# Search for connected ESL Blaster if required
if (port == "0"):
    blaster_port = tx.search_esl_blaster()
    if (blaster_port == "0"):
        exit()

# Open image file
image = imread(sys.argv[2])
width = image.shape[1]
height = image.shape[0]
if width > 208 or height > 112:
    print("Image should be 208*112 pixels or less.")
    exit()

# Get PLID from barcode string
PLID = pr.get_plid(sys.argv[3])

page = int(sys.argv[4])
if page < 0 or page > 15:
    print("Page can only be between 0 and 15.")
    exit()

if arg_count > 5:
    pos_x = int(sys.argv[5])
else:
    pos_x = 0
if arg_count > 6:
    pos_y = int(sys.argv[6])
else:
    pos_y = 0

bits = []
size_raw = width * height

# Convert image to 1-bit
pixels = []
for row in image:
    for rgb in row:
        pixels.append(int(round((0.21 * rgb[0] + 0.72 * rgb[1] + 0.07 * rgb[2]) / 255)))

print("Compressing %i pixels..." % size_raw)

# First pixel
compressed = []
run_pixel = pixels[0]
run_count = 1
compressed.append(run_pixel)
for pixel in pixels[1:]:
    if pixel == run_pixel:
        # Add to run
        run_count += 1
    else:
        # Record run
        record_run(run_count)
        run_count = 1
        run_pixel = pixel

# Record last run
if run_count > 1:
    record_run(run_count)

size_compressed = len(compressed)

# Decide on compression or not
if size_compressed < size_raw:
    print("Compression ratio: %.1f%%" % (100 - ((size_compressed * 100) / float(size_raw))))
    data = compressed
    compression_type = 2
else:
    print("Compression ratio suxx, using raw data")
    data = pixels
    compression_type = 0

# Pad data to multiple of bits_per_frame
bits_per_frame = 20 * 8
data_size = len(data)
padding = bits_per_frame - (data_size % bits_per_frame)
for b in range(0, padding):
    data.append(0)

padded_data_size = len(data)
frame_count = padded_data_size / bits_per_frame

frames = []

# Ping frame
frames.append(pr.make_ping_frame(PLID, 150))

print("Generating %i data frames..." % frame_count)

# Parameters frame
frame = pr.make_mcu_frame(PLID, 0x05)
pr.append_word(frame, data_size / 8)    # Byte count
frame.append(0x00)                      # Unused
frame.append(compression_type)
frame.append(page)
pr.append_word(frame, width)
pr.append_word(frame, height)
pr.append_word(frame, pos_x)
pr.append_word(frame, pos_y)
pr.append_word(frame, 0x0000)   # Keycode
frame.append(0x88)              # 0x80 = update, 0x08 = set base page
pr.append_word(frame, 0x0000)   # Enabled pages (bitmap)
frame.extend([0x00, 0x00, 0x00, 0x00])
pr.terminate_frame(frame, 1)
frames.append(frame)

# Data frames
i = 0
for fr in range(0, frame_count):
    frame = pr.make_mcu_frame(PLID, 0x20)
    pr.append_word(frame, fr)
    for by in range(0, 20):
        v = 0
        # Bit string to byte
        for bi in range(0, 8):
            v <<= 1
            v += data[i + bi]
        frame.append(v)
        i += 8
    pr.terminate_frame(frame, 1)
    frames.append(frame)

# Refresh frame
frames.append(pr.make_refresh_frame(PLID))

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
print("Done. Please allow a few seconds for the ESL to refresh.")
