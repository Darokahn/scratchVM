all: hello

hello: source/log.c source/scratch.c source/programData.c source/definitions.c
	gcc $^ -lm -g -fsanitize=address
