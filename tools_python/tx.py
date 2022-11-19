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
        if (len(test) >= 10):
            print("Found ESL Blaster on " + comport)
            return True
        else:
           	print("Timeout on " + comport)
           	return False
    except serial.SerialException:
        #print("SerialException on " + comport)	# Debug
        return False

def search_esl_blaster():
	found = False

	for n in range(1, 10):
		comport = "COM" + str(n)
		if try_serialport(comport):
			found = True
			break

	if found == False:
		for n in range(1, 10):
			comport = "/dev/ttyACM" + str(n)
			if try_serialport(comport):
				found = True
				break

	if found == False:
		print("Could not find ESL Blaster.")
		return "0"
	else:
		return comport

def transmit_serial(frames, port):
    ser = serial.Serial(port, 57600, timeout = 10)    # 10s timeout for read
    ser.reset_input_buffer()
    frame_count = len(frames)
    i = 1
    for fr in frames:
        data_size = len(fr) - 2
        repeats = fr[-2] + (fr[-1] * 256)
        if repeats > 255:	# Cap to one byte for the simple serial transmitter
            repeats = 255
        print("Transmitting frame %u/%u, length %u, repeated %u times." % (i, frame_count, data_size, repeats))

        ba = bytearray()
        ba.append(data_size)
        ba.append(repeats)
        for b in range(0, data_size):
            ba.append(fr[b])

        ser.write(ba)
        ser.flush()
        ser.read_until('A')
        i += 1

    ser.close()

def transmit_esl_blaster(frames, port):
    ser = serial.Serial(port, 57600, timeout = 10)    # 10s timeout for read
    ser.reset_input_buffer()
    frame_count = len(frames)
    i = 1
    
    # DEBUG
    #f = open("out_bin.txt", "w")

    for fr in frames:
        data_size = len(fr) - 2
        repeats = fr[-2] + (fr[-1] * 256)
        print("Transmitting frame %u/%u, length %u, repeated %u times." % (i, frame_count, data_size, repeats))

        ba = bytearray()
        ba.append(76)	# L:Load
        ba.append(data_size)
        ba.append(30)   # 30*50 = 1500 timer ticks between repeats
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

    ser.close()
