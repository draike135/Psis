# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -lncurses -lzmq

# Source files
SOURCES = game-server.c aliens.c astronauts.c table.c astronaut-client.c outer-space-display.c
HEADERS = aliens.h astronaut.h table.h remote-char.h
OBJECTS = $(SOURCES:.c=.o)

# Output binary names
SERVER = game-server
CLIENT = astronaut-client
DISPLAY = outer-space-display

# Default target
all: $(SERVER) $(CLIENT) $(DISPLAY)

# Server executable
$(SERVER): game-server.o aliens.o astronauts.o table.o
	$(CC) $^ -o $@ $(CFLAGS)

# Astronaut client executable
$(CLIENT): astronaut-client.o
	$(CC) $^ -o $@ $(CFLAGS)

# Outer space display executable
$(DISPLAY): outer-space-display.o aliens.o astronauts.o table.o
	$(CC) $^ -o $@ $(CFLAGS)

# Generic rule for creating object files
%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS)

# Clean up build files
clean:
	rm -f $(OBJECTS) $(SERVER) $(CLIENT) $(DISPLAY)
