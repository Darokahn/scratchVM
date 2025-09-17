all: main

main: source/log.c source/scratch.c source/programData.c source/definitions.c source/main.c source/graphics.c
	gcc $^ -lm -g -fsanitize=address

# Rule to build definitions.c using the generator
source/definitions.c: source/generateDefinitions
	./$< > $@

# Rule to build the generator program
source/generateDefinitions: source/generateDefinitions.c
	gcc -g -o $@ $< -lm
