CC = gcc
CFLAGS = -Wall
BUILDDIR = build
TARGET = main
OBJECTS = $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard *.c))

all: dir $(BUILDDIR)/$(TARGET)

dir:
	mkdir -p $(BUILDDIR)
	touch $(BUILDDIR)/fileSave

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall -o $@ -lrt

clean:
	rm -rf $(BUILDDIR)