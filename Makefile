CC = g++

# Update these paths as necessary to match your installation
SDL_LIB = -LD:\MinGW\lib -lSDLmain -lSDL
SDL_INCLUDE = -ID:\MinGW\include

# If your compiler is a bit older you may need to change -std=c++11 to -std=c++0x
CXXFLAGS = -Wall -c $(SDL_INCLUDE)
LDFLAGS = -lmingw32 -mwindows -mconsole $(SDL_LIB)
EXE = gdkGBA

all: $(EXE)

$(EXE): sdl.o main.o arm.o arm_inst.o arm_mem.o arm_utils.o io.o dma.o video.o 
	$(CC) $< $(LDFLAGS) -o $@

sdl.o main.o arm.o arm_inst.o arm_mem.o arm_utils.o io.o dma.o video.o: sdl.c main.c arm.c arm_inst.c arm_mem.c arm_utils.c io.c dma.c video.c
	$(CC) $(CXXFLAGS) $< -o $@

clean:
	del *.o && del $(EXE)