import serial

location = '/dev/ttyUSB3'
rate = 19200

with serial.Serial(location, rate, xonxoff=False) as port:
    with open('program.bin', 'rb') as f:
        data = f.read()
        port.write(data)
