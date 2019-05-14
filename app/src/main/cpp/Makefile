TARGET   = gearsvk
CLASSES  = gears_renderer gear
SOURCE   = $(TARGET).c $(CLASSES:%=%.c)
OBJECTS  = $(TARGET).o $(CLASSES:%=%.o)
HFILES   = $(CLASSES:%=%.h)
OPT      = -O2 -Wall
CFLAGS   = $(OPT) -I. -I$(VULKAN_SDK)/include `sdl2-config --cflags` -DA3D_GL2
LDFLAGS  = -L$(VULKAN_SDK)/lib -lvulkan -lGLEW -lGL -lGLU -L/usr/lib `sdl2-config --libs` -Llibpak -lpak -Ltexgz -ltexgz -La3d -la3d -Llibexpat/expat/lib -lexpat -lm -lpthread
CCC      = gcc

all: $(TARGET) libpak texgz a3d libexpat

$(TARGET): $(OBJECTS) libpak texgz a3d libexpat
	$(CCC) $(OPT) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY: libpak texgz a3d libexpat

libpak:
	$(MAKE) -C libpak

texgz:
	$(MAKE) -C texgz

a3d:
	$(MAKE) -C a3d -f Makefile.sdl

libexpat:
	$(MAKE) -C libexpat/expat/lib

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)
	$(MAKE) -C libpak clean
	$(MAKE) -C texgz clean
	$(MAKE) -C a3d -f Makefile.sdl clean
	$(MAKE) -C libexpat/expat/lib clean

$(OBJECTS): $(HFILES)
