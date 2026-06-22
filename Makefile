all: esp32 upload

port=/dev/ttyUSB0
baud=921600
clean=
#fqbn=arduino:avr:uno
fqbn=esp32:esp32:esp32
desktop: source/*.c
	gcc $^ -lm -lSDL2 -g3 -fsanitize=address,leak,undefined -O0 -Wextra -Wall -Wno-missing-braces -Wno-old-style-declaration

esp32:
	arduino-cli compile $(clean) -v --fqbn $(fqbn) --library ./arduino/libraries/TFT_eSPI --output-dir ./arduino/build/ arduino/

upload:
	arduino-cli upload -p $(port) --fqbn $(fqbn) arduino/

monitor:
	screen $(port) $(baud)
