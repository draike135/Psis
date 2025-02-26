# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -lncurses -lzmq -lpthread -lprotobuf-c

# Header files
HEADERS = aliens.h astronaut.h table.h remote-char.h globals.h score.pb-c.h

# Source files
SOURCES_SERVER = game-server.c aliens.c astronauts.c table.c globals.c score.pb-c.c
SOURCES_CLIENT = astronaut-client.c astronauts.c globals.c
SOURCES_DISPLAY = outer-space-display.c aliens.c astronauts.c table.c globals.c
SOURCES_DISPLAY_CLIENT = astronaut-display-client.c aliens.c astronauts.c table.c globals.c

# Object files
OBJECTS_SERVER = $(SOURCES_SERVER:.c=.o)
OBJECTS_CLIENT = $(SOURCES_CLIENT:.c=.o)
OBJECTS_DISPLAY = $(SOURCES_DISPLAY:.c=.o)
OBJECTS_DISPLAY_CLIENT = $(SOURCES_DISPLAY_CLIENT:.c=.o)

# Output binaries
SERVER = game-server
CLIENT = astronaut-client
DISPLAY = outer-space-display
DISPLAY_CLIENT = astronaut-display-client

# Default target to build all programs
all: $(SERVER) $(CLIENT) $(DISPLAY) $(DISPLAY_CLIENT)

# Server executable
$(SERVER): $(OBJECTS_SERVER)
	$(CC) $^ -o $@ $(CFLAGS)

# Client executable
$(CLIENT): $(OBJECTS_CLIENT)
	$(CC) $^ -o $@ $(CFLAGS)

# Outer space display executable
$(DISPLAY): $(OBJECTS_DISPLAY)
	$(CC) $^ -o $@ $(CFLAGS)

# Astronaut display client executable
$(DISPLAY_CLIENT): $(OBJECTS_DISPLAY_CLIENT)
	$(CC) $^ -o $@ $(CFLAGS)

# Rule to compile .c files to .o files
%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@ $(CFLAGS)

# Clean up build files
clean:
	rm -f $(OBJECTS_SERVER) $(OBJECTS_CLIENT) $(OBJECTS_DISPLAY) $(OBJECTS_DISPLAY_CLIENT) $(SERVER) $(CLIENT) $(DISPLAY) $(DISPLAY_CLIENT)
