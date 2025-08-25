all: hello

hello: source/main.c source/log.c source/scratch.c
	gcc source/main.c source/log.c source/scratch.c -lm -g -fsanitize=address
