CC = gcc
CFLAGS = -Wall
BUILDDIR = build
OBJECTS = $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard *.c))
TARGETS = $(patsubst %.c, $(BUILDDIR)/%, $(wildcard *.c))

all: dir $(TARGETS)

dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGETS): %: %.o
	$(CC) $^ $(CFLAGS) -o $@ -lrt

clean:
	rm -rf $(BUILDDIR)