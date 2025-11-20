all: main

main: source/*.c
	cp webConverter/upload/definitions.c source/definitions.c
	gcc $^ -lm -lSDL2 -g3 -fsanitize=address,leak,undefined -O0
