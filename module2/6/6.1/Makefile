CC = gcc
CFLAGS = -Wall -I.
TARGET = main
LIBRARY = libmanual.a
BUILDDIR = build

SRCS = main.c menu.c manual.c
OBJS = $(addprefix $(BUILDDIR)/, main.o menu.o)
LIB_OBJ = $(BUILDDIR)/manual.o

.PHONY: all clean directories

all: directories $(BUILDDIR)/$(TARGET)

directories:
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/$(LIBRARY): $(LIB_OBJ)
	ar rcs $@ $^

$(BUILDDIR)/$(TARGET): $(OBJS) $(BUILDDIR)/$(LIBRARY)
	$(CC) $^ -L$(BUILDDIR) -lmanual -o $@

$(BUILDDIR)/main.o: main.c menu.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/menu.o: menu.c menu.h manual.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/manual.o: manual.c manual.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)