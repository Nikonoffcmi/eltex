CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared
TARGET = main
LIBNAME = libmanual.so
BUILDDIR = build

.PHONY: all clean directories

all: directories $(BUILDDIR)/$(LIBNAME) $(BUILDDIR)/$(TARGET)

directories:
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/$(LIBNAME): $(BUILDDIR)/manual.o $(BUILDDIR)/menu.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/$(TARGET): $(BUILDDIR)/main.o
	$(CC) -L$(BUILDDIR) -lmanual -Wl,-rpath=$(BUILDDIR) -o $@ $^

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)