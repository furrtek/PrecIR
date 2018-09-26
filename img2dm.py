# Converts, compresses and transmits images to dot matrix ESLs
# 2018 furrtek - furrtek.org
# See LICENSE

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

def append_crc(frame):
    result = 0x8408
    poly = 0x8408

    for by in frame:
        result ^= by
        for bi in range(0, 8):
            if (result & 1):
                result >>= 1
                result ^= poly
            else:
                result >>= 1

    frame.append(result & 255)
    frame.append((result / 256) & 255)

def make_mcu_frame(cmd):
    frame = [0x85, PLID[3], PLID[2], PLID[1], PLID[0], 0x34, 0x00, 0x00, 0x00, cmd]
    return frame

def append_word(frame, value):
    frame.append(value / 256)
    frame.append(value & 255)

def usage():
    print("Usage:")
    print("img2dm.py image barcode page x y")
    print("  image: image file")
    print("  barcode: 17-character barcode data")
    print("  page: page number to update (0~15)")
    print("  x y: top-left position of image")
    exit()

arg_count = len(sys.argv)
if arg_count < 4:
    usage()

# Open image file
image = imread(sys.argv[1])
width = image.shape[1]
height = image.shape[0]
if width != 208 or height != 112:
    print("Image should be 208*112 pixels or less.")
    exit()

# Get PLID
PLID = [0] * 4
barcode = sys.argv[2]
if len(barcode) != 17:
    usage()
id_value = int(barcode[2:7]) + (int(barcode[7:12]) << 16)
PLID[0] = (id_value >> 8) & 0xFF
PLID[1] = id_value & 0xFF
PLID[2] = (id_value >> 24) & 0xFF
PLID[3] = (id_value >> 16) & 0xFF

page = int(sys.argv[3])
if page < 0 or page > 15:
    print("Page can only be between 0 and 15.")
    exit()

if arg_count > 4:
    pos_x = int(sys.argv[4])
else:
    pos_x = 0
if arg_count > 5:
    pos_y = int(sys.argv[5])
else:
    pos_y = 0

pixels = []
compressed = []
bits = []

size_raw = width * height

for row in image:
    for rgb in row:
        pixels.append(int(round((0.21 * rgb[0] + 0.72 * rgb[1] + 0.07 * rgb[2]) / 255)))

print("Compressing %i pixels..." % len(pixels))

# First pixel
run_pixel = pixels[0]
run_count = 1
compressed.append(run_pixel)
c = 0
for pixel in pixels[1:]:
    if pixel == run_pixel:
        # Add to count
        run_count += 1
    else:
        record_run(run_count)

        run_count = 1
        run_pixel = pixel

# Record last run
if run_count > 1:
    record_run(run_count)

size_compressed = len(compressed)

if size_compressed < size_raw:
    print("Compression ratio: %.1f%%" % (100 - ((size_compressed * 100) / float(size_raw))))
    data = compressed
    compression_type = 2
else:
    print("Compression ratio suxx, using raw mode")
    data = pixels
    compression_type = 0

# Pad to multiple of bits_per_frame
bits_per_frame = 20 * 8
padding = bits_per_frame - (len(data) % bits_per_frame)
for b in range(0, padding):
    data.append(0)

data_size = len(data)
frame_count = data_size / bits_per_frame

print("Generating %i frames..." % frame_count)

frames = []

# Parameters frame
frame = make_mcu_frame(0x05)
byte_count = data_size / 8
append_word(frame, byte_count)
frame.append(0x00)
frame.append(compression_type)
frame.append(0x01)   # PAGE !
append_word(frame, width)
append_word(frame, height)
append_word(frame, pos_x)
append_word(frame, pos_y)
frame.extend([0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
append_crc(frame)
frames.append(frame)

i = 0
for fr in range(0, frame_count):
    frame = make_mcu_frame(0x20)
    append_word(frame, fr)
    for by in range(0, 20):
        v = 0
        for bi in range(0, 8):
            v <<= 1
            v += data[i + bi]
        frame.append(v)
        i += 8
    append_crc(frame)
    frames.append(frame)

f = open("out.txt", "w")    # DEBUG

for fr in frames:
    for b in fr:
        f.write(format(b, '02X') + " ")
    f.write("\n")
