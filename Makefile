TARGET = gdkGBA
LIBS = -lm -lSDL2
CC = gcc
CFLAGS = -std=c99 -g -Wall -Ofast

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS += $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h) $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall  -Ofast $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)