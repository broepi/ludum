
.PHONY: build, rebuild, clean

PROJECT = hamsibro
CC = g++
COMPFLAGS = -Iinclude -I/usr/include/SDL2 -Wno-write-strings -fpermissive
LIBS = -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2
LINKFLAGS = -Llibs/amd64
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

