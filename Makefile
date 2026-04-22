all: main

main: source/*.c
	cp definitions.c source/definitions.c
	cp program.bin source/program.bin
	gcc $^ -lm -lSDL2 -g3 -fsanitize=address,leak,undefined -O0 -Wextra -Wall -Wno-missing-braces -Wno-old-style-declaration
	#gcc $^ -lm -lSDL2 -g3 -O0 -Wextra -Wall -Wno-missing-braces -Wno-old-style-declaration
