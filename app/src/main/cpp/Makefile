TARGET   = gearsvk
CLASSES  = gears_renderer gear
SOURCE   = $(TARGET).c $(CLASSES:%=%.c)
OBJECTS  = $(TARGET).o $(CLASSES:%=%.o)
HFILES   = $(CLASSES:%=%.h)
OPT      = -O2 -Wall
CFLAGS   = $(OPT) -I. -I$(VULKAN_SDK)/include `sdl2-config --cflags` -DA3D_GL2
LDFLAGS  = -Llibvkk -lvkk -L$(VULKAN_SDK)/lib -lvulkan `sdl2-config --libs` -Llibpak -lpak -Ltexgz -ltexgz -La3d -la3d -Llibcc -lcc -Llibexpat/expat/lib -lexpat -lm -lpthread -lz -lpng
CCC      = gcc

all: $(TARGET) libcc libpak texgz a3d libvkk libexpat

$(TARGET): $(OBJECTS) libcc libpak texgz a3d libvkk libexpat
	$(CCC) $(OPT) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY: libcc libpak texgz a3d libvkk libexpat

libcc:
	$(MAKE) -C libcc

libpak:
	$(MAKE) -C libpak

texgz:
	$(MAKE) -C texgz

a3d:
	$(MAKE) -C a3d -f Makefile.sdl

libvkk:
	$(MAKE) -C libvkk

libexpat:
	$(MAKE) -C libexpat/expat/lib

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)
	$(MAKE) -C libcc clean
	$(MAKE) -C libpak clean
	$(MAKE) -C texgz clean
	$(MAKE) -C a3d -f Makefile.sdl clean
	$(MAKE) -C libvkk clean
	$(MAKE) -C libexpat/expat/lib clean

$(OBJECTS): $(HFILES)
