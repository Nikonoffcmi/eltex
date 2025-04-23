CC = gcc
CFLAGS = -Wall
BUILDDIR = build
OBJECTS = $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard *.c))
TARGETS = $(patsubst %.c, $(BUILDDIR)/%, $(wildcard *.c))

all: dir $(TARGETS)

dir:
	mkdir -p $(BUILDDIR)
	touch $(BUILDDIR)/chat

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGETS): %: %.o
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -rf $(BUILDDIR)