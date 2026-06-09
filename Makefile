CC = cc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Isrc
LDLIBS = -lm -lpthread -lreadline

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = soshell

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

src/%.o: src/%.c src/soshell.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean