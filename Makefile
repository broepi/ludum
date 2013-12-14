
.PHONY: build, rebuild, clean

PROJECT = mygame
CC = g++
COMPFLAGS = $(shell pkg-config --cflags sdl2)
LIBS = $(shell pkg-config --libs sdl2)
LINKFLAGS = 
OBJECTS = $(subst src,build,$(subst .cpp,.o,$(wildcard src/*.cpp)))

build : build/$(PROJECT)

rebuild : clean build

clean:
	-rm build/*.o
	-rm build/$(PROJECT)

build/$(PROJECT) : $(OBJECTS)
	$(CC) -o $@ $(LINKFLAGS) $^ $(LIBS)

build/%.o : src/%.cpp
	$(CC) -o $@ $(COMPFLAGS) -c $^

