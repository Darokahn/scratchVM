all: main

main: source/*.c
	cp webConverter/upload/definitions.c source/definitions.c
	cp webConverter/upload/programData.bin source/program.bin
	gcc $^ -lm -lSDL2 -g3 -fsanitize=address,leak,undefined -O0 -Wextra -Wall -Wno-missing-braces -Wno-old-style-declaration
