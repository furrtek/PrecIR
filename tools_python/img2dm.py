# Converts, compresses and transmits images to dot matrix ESLs
# 2018 furrtek - furrtek.org
# See LICENSE

import pr
import tx
from imageio import imread
import sys

bytes_per_frame = 20
bits_per_frame = bytes_per_frame * 8

def image_convert(image, color_pass):
	pixels = []
	for row in image:
		for rgb in row:
			luma = 0.21 * rgb[0] + 0.72 * rgb[1] + 0.07 * rgb[2]
			if color_pass:
				pixels.append(0 if luma >= 0.33 and luma < 0.66 else 1)	# 0 codes color (anything mid grey)
			else:
				pixels.append(0 if luma < 0.5 else 1)	# 0 codes black
	        	
	return pixels

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
    print("img2dm.py port image barcode (page color x y)\n")
    print("  port: serial port name (0 for ESL Blaster)")
    print("  image: image file")
    print("  barcode: 17-character barcode data")
    print("  page: page number to update (0~15), default: 0")
    print("  color: 0:Black and white only, 1:Color-capable ESL, default: 0")
    print("  x y: top-left position of image, default: 0 0")
    exit()

arg_count = len(sys.argv)
if arg_count < 4:
    usage()

if arg_count >= 5:
	page = int(sys.argv[4])
	if page < 0 or page > 15:
	    print("Page must be between 0 and 15.")
	    exit()
else:
	page = 0

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
# Medium size is 208*112
print("Image is %i*%i, please make sure that this is equal or less than the ESL's display size." % (width, height))

# Get PLID from barcode string
PLID = pr.get_plid(sys.argv[3])

color_mode = int(sys.argv[5]) if arg_count >= 6 else 0
pos_x = int(sys.argv[6]) if arg_count >= 7 else 0
pos_y = int(sys.argv[7]) if arg_count >= 8 else 0

# First pass for black and white
pixels = image_convert(image, 0)

if color_mode:
	# Append second pass for color  if needed
	pixels += image_convert(image, 1)

size_raw = len(pixels)
print("Compressing %i bits..." % size_raw)

# First pixel
bits = []
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
# size_compressed = size_raw # Disable compression
if size_compressed < size_raw:
    print("Compression ratio: %.1f%%" % (100 - ((size_compressed * 100) / float(size_raw))))
    data = compressed
    compression_type = 2
else:
    print("Compression ratio suxx, using raw data")
    data = pixels
    compression_type = 0

# Pad data to multiple of bits_per_frame
data_size = len(data)
padding = bits_per_frame - (data_size % bits_per_frame)
for b in range(0, padding):
    data.append(0)

padded_data_size = len(data)
frame_count = padded_data_size // bits_per_frame

print("Data size: %i (%i frames)" % (data_size, frame_count))

frames = []

# Wake-up ping frame
frames.append(pr.make_ping_frame(PLID, 400))

print("Generating frames...")

# Parameters frame
frame = pr.make_mcu_frame(PLID, 0x05)
pr.append_word(frame, data_size // 8)	# Total byte count for group
frame.append(0x00)              # Unused
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
    for by in range(0, bytes_per_frame):
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
f = open("frames.txt", "w")
for fr in frames:
    for b in fr:
        f.write(format(b, '02X') + " ")
    f.write("\n")

input("Place ESL in front of transmitter and press any key.")

# Send data to IR transmitter
if (port == "0"):
    tx.transmit_esl_blaster(frames, blaster_port)
else:
    tx.transmit_serial(frames, port)
print("Done. Please allow a few seconds for the ESL to refresh.")
