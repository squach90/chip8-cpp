# Compiler et flags
CC = gcc
CFLAGS = -Wall -O2 -Iinclude `sdl2-config --cflags`
LDFLAGS = -lm `sdl2-config --libs`

# Sources et cible
SRC = src/main.c src/cpu.c src/display.c src/controller.c
TARGET = chip8

# Règle par défaut
all: $(TARGET)

# Compilation du programme
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Nettoyage
clean:
	rm -f $(TARGET)
