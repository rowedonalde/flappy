.PHONY: clean

all: flappy.c
	@gcc -o flappy flappy.c `sdl2-config --cflags --libs`

clean:
	@rm -f flappy
