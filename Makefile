all: hello

hello: source/main.c source/log.c source/scratch.c
	gcc $^ -lm -g -fsanitize=address
