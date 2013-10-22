CC := gcc
FLAGS := -W -Wall -g -ansi -std=c99 -pedantic
LIBS := -lm -lSDL -lSDLmain

all: mk_dir bin/parser bin/extract_heightmap bin/display

bin/parser: src/parser.c
	$(CC) $(FLAGS) $^ -o $@

bin/extract_heightmap: src/extract_heightmap.c
	$(CC) $(FLAGS) $^ -o $@

bin/display: src/display.c
	$(CC) $(FLAGS) $(LIBS) $^ -o $@

mk_dir:
	mkdir -p bin

clean:
	rm -f bin/parser bin/extract_heightmap bin/display
