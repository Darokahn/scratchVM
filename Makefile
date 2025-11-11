all: main

main: source/*.c
	gcc $^ -lm -lSDL2 -g -fsanitize=address,leak,undefined -O0 -Wall -Wextra -Wpedantic
