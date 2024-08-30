PROJECT=server
SOURCES=
LIBRARY=nope
INCPATHS=include
LIBPATHS=.
LDFLAGS=-std=c++1z -g
CFLAGS=-c -Wall -g
CC=g++
# Automatic generation of some important lists
OBJECTS=$(SOURCES:.c=.o)
INCFLAGS=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS=$(foreach TMP,$(LIBPATHS),-L$(TMP))

# Set up the output file names for the different output types
BINARY=$(PROJECT)

all: Client commands main
	$(CC) $(LDFLAGS) -o client Client.o commands.o main.o

Client:
	$(CC) $(LDFLAGS) -c Client.cpp -o Client.o

commands:
	$(CC) $(LDFLAGS) -c commands.cpp -o commands.o

main:
	$(CC) $(LDFLAGS) -c main.cpp -o main.o

distclean: clean
	rm -f $(BINARY)

run_client: all
	./client

clean:
	rm -f $(OBJECTS) Client.o commands.o main.o client

