CC = gcc
CFLAGS = -Wall -fPIC -I./src
LDFLAGS = -ldl -lm
TARGET = main
PLUGINS_DIR = plugins
BUILDDIR = build
PLUGINS_BUILD = $(BUILDDIR)/plugins
SRC_DIR = src

.PHONY: all clean dir plugins

all: dir plugins $(BUILDDIR)/$(TARGET)

dir:
	mkdir -p $(BUILDDIR)
	mkdir -p $(PLUGINS_BUILD)

plugins: $(PLUGINS_BUILD)/add.so \
         $(PLUGINS_BUILD)/sub.so \
         $(PLUGINS_BUILD)/mul.so \
         $(PLUGINS_BUILD)/div.so \
         $(PLUGINS_BUILD)/pow.so \
         $(PLUGINS_BUILD)/sqrt.so

$(PLUGINS_BUILD)/%.so: $(PLUGINS_DIR)/%.c
	$(CC) $(CFLAGS) -shared -o $@ $< -lm

$(BUILDDIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

OBJECTS = $(BUILDDIR)/main.o $(BUILDDIR)/calculator.o

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILDDIR)/