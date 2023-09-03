# Procedures related to the IR transmitters only
# 2019 furrtek - furrtek.org
# See LICENSE

import serial

def try_serialport(comport):
	try:
		ser = serial.Serial(comport, 57600, timeout = 1)    # 1s timeout for read
		ser.write("?".encode())
		ser.flush()
		test = ser.read_until("ESLBlaster")
		ser.close()
		if (len(test) >= 12):
			hw_ver = chr(test[10])
			fw_ver = int(chr(test[11]))
			print("Found ESL Blaster (HW %s, FW V%d) on %s"  % (hw_ver, fw_ver, comport))
			return [True, hw_ver, fw_ver]
		else:
			print("Timeout on " + comport)
			return [False, 0, 0]
	except serial.SerialException:
		#print("SerialException on " + comport)	# Debug
		return [False, 0, 0]

def search_esl_blaster():
	found = False

	# Windows
	for n in range(1, 10):
		comport = "COM" + str(n)
		result = try_serialport(comport)
		if result[0]:
			found = True
			break

	# Linux
	if found == False:
		for n in range(0, 10):
			comport = "/dev/ttyACM" + str(n)
			result = try_serialport(comport)
			if result[0]:
				found = True
				break

	if found == False:
		print("Could not find ESL Blaster.")
	else:
		result[0] = comport

	return result
	
def show_progress(i, total, size, repeats, pp16):
    if repeats > 100:
        print("Transmitting wake-up frames, please wait...")	# Lots of repeats certainly means this is a wake-up frame
    else:
        print("Transmitting frame %u/%u using %s, length %u, repeated %u times." % (i, total, "PP16" if pp16 else "PP4", size, repeats), end="\r", flush=True)

def transmit_serial(frames, port):
    ser = serial.Serial(port, 57600, timeout = 10)    # 10s timeout for read
    ser.reset_input_buffer()
    frame_count = len(frames)
    i = 1
    for fr in frames:
        data_size = len(fr) - 2
        repeats = fr[-2] + (fr[-1] * 256)
        if repeats > 255:
            repeats = 255	# Cap to one byte for the simple serial transmitter
        show_progress(i, frame_count, data_size, repeats, 0)

        ba = bytearray()
        ba.append(data_size)
        ba.append(repeats)
        for b in range(0, data_size):
            ba.append(fr[b])

        ser.write(ba)
        ser.flush()
        ser.read_until('A')
        i += 1
    print("")
    ser.close()

def transmit_esl_blaster(frames, pp16, port):
    ser = serial.Serial(port, 57600, timeout = 10)    # 10s timeout for read
    ser.reset_input_buffer()
    frame_count = len(frames)
    i = 1
    
    # DEBUG
    #f = open("out_bin.txt", "w")

    for fr in frames:
        data_size = len(fr) - 2
        repeats = fr[-2] + (fr[-1] * 256)

        if repeats > 32767:
            repeats = 32767	# Cap to 15 bits because FW V2 uses bit 16 to indicate PP16 protocol
        #if repeats > 100:
        #    print("Transmitting wake-up frames, please wait...")	# Lots of repeats certainly means this is a wake-up frame
        #else:
        #	print("Transmitting frame %u/%u using %s, length %u, repeated %u times." % (i, frame_count, "PP16" if pp16 else "PP4", data_size, repeats), end="\r", flush=True)
        show_progress(i, frame_count, data_size, repeats, pp16)

        if pp16:
        	repeats |= 0x8000

        ba = bytearray()
        ba.append(76)	# L:Load
        ba.append(data_size)
        ba.append(10)   # 30*50 = 1500 timer ticks between repeats
        ba.append(repeats & 255)
        ba.append((repeats // 256) & 255)
        for b in range(0, data_size):
            ba.append(fr[b])
        ba.append(84)	# T:Transmit

        # DEBUG
        #for b in ba:
        #    f.write(format(b, '02X') + " ")
        #f.write("\n")

        ser.write(ba)
        ser.flush()
        ser.read_until(b'K')	# Wait for transmit done
        i += 1
    print("")
    ser.close()
