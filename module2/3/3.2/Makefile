CC = gcc
CFLAGS = -Wall
TARGET = main

BUILDDIR = build

.PHONY: all clean dir

all: dir $(BUILDDIR)/$(TARGET)

dir:
	mkdir -p $(BUILDDIR)

OBJECTS = $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

$(BUILDDIR)/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall -o $@ -lm

clean:
	-rm -f $(BUILDDIR)/*.o
	-rm -f $(BUILDDIR)/$(TARGET)

