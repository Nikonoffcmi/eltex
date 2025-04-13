CC = gcc
CFLAGS = -Wall
BUILDDIR = build
TARGET = main
OBJECTS = $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard *.c))

all: dir $(BUILDDIR)/$(TARGET)

dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall -o $@

clean:
	rm -rf $(BUILDDIR)