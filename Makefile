all: main

main: source/*.c
	gcc $^ -lm -lSDL2 -g3 -fsanitize=address,leak,undefined -O0
