OBJS   = btext.o main.o
TARGET = coloradjust
CFLAGS = `sdl-config --cflags` -pedantic -Wall -Wextra -std=gnu99 -ggdb
#CFLAGS = `sdl-config --cflags` -pedantic -Wall -Wextra -std=gnu99
LDFLAGS = `sdl-config --libs`

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
