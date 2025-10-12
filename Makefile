all: main

main: source/*.c
	gcc $^ -lm -lSDL2 -g -fsanitize=address

# Rule to build definitions.c using the generator
#source/definitions.c: source/gdef
#	./$< > $@

# Rule to build the generator program
source/gdef: source/generateDefinitions.c
	gcc -g -o $@ $< -lm -fsanitize=address
