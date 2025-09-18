all: main

main: source/scratch.c source/programData.c source/definitions.c source/main.c source/graphics.c source/externalDefinitions.c
	gcc $^ -lm -lSDL2 -g -fsanitize=address

# Rule to build definitions.c using the generator
#source/definitions.c: source/gdef
	#./$< > $@

# Rule to build the generator program
source/gdef: source/generateDefinitions.c
	gcc -g -o $@ $< -lm -fsanitize=address
