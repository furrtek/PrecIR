# Procedures related to the IR transmitters only
# 2019 furrtek - furrtek.org
# See LICENSE

import serial

def search_esl_blaster():
    found = 0
    for n in range(1, 11):
        comport = "COM" + str(n)
        try:
            ser = serial.Serial(comport, 57600, timeout = 1)    # 1s timeout for read
            ser.write('?')
            ser.flush()
            test = len(ser.read_until("ESLBlaster"))
            ser.close()
            if (test == 10):
                print("Found ESL Blaster on " + comport)
                found = 1
                break
        except serial.SerialException:
            continue

    if (found == 0):
        print("Could not find ESL Blaster.")
        return "0"
    else:
        return comport

def transmit_serial(frames, port):
    ser = serial.Serial(port, 57600, timeout = 5)    # 5s timeout for read
    ser.reset_input_buffer()
    frame_count = len(frames)
    i = 1
    for fr in frames:
        data_size = len(fr) - 1
        repeats = fr[-1]
        print("Transmitting frame %u/%u, length %u, repeated %u times." % (i, frame_count, data_size, repeats))
    
        ba = bytearray()
        ba.append(data_size)
        ba.append(repeats)
        for b in range(0, len(fr) - 1):
            ba.append(fr[b])
        ser.write(ba)
        ser.flush()
        ser.read_until('A')
        i += 1
    
    ser.close()

def transmit_esl_blaster(frames, port):
    ser = serial.Serial(port, 57600, timeout = 5)    # 5s timeout for read
    ser.reset_input_buffer()
    frame_count = len(frames)
    i = 1
    for fr in frames:
        data_size = len(fr) - 1
        repeats = fr[-1]
        print("Transmitting frame %u/%u, length %u, repeated %u times." % (i, frame_count, data_size, repeats))
    
        ba = bytearray()
        ba.append('L')
        ba.append(data_size)
        ba.append(repeats)
        for b in range(0, len(fr) - 1):
            ba.append(fr[b])
        ba.append('T')
        ser.write(ba)
        ser.flush()
        ser.read_until('A')
        i += 1
    
    ser.close()
