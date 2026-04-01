arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 arduino/

python3 sendBytes.py -r source/program.bin --port /dev/ttyUSB0
