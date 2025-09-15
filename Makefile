all: main

main: source/log.c source/scratch.c source/programData.c source/definitions.c source/main.c source/graphics.c
	gcc $^ -lm -g -fsanitize=address
