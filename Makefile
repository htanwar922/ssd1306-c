# Makefile for building a C project with cl.exe and nmake
CC=cl
CFLAGS=/nologo /W3 /Zi /Od
LDFLAGS=/nologo

TARGET=main.exe

INCLUDE_DIR=include

SRCS=main.c \
	fonts/font_8x9.c \
	fonts/font_16x8.c \
	src/layout.c \
	src/udp.c \
	src/ssd1306.c \
	src/i2c.c \

OBJS=$(SRCS:.c=.obj)

CFLAGS=$(CFLAGS) /I$(INCLUDE_DIR)
# LDFLAGS=$(LDFLAGS) hidapi.lib

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking..."
	@$(CC) $(LDFLAGS) /Fe$@ $(OBJS)
	@echo "Build complete: $@"

$(OBJS):
	@echo "Compiling $(subst .obj,.c,$@)..."
	@$(CC) $(CFLAGS) /c $(subst .obj,.c,$@) /Fo$@

clean:
	@echo "Cleaning up..."
	@if exist "$(TARGET:/=\)" del /Q $(TARGET:/=\)
	@del /Q $(OBJS:/=\)
	@echo "Clean complete."

.PHONY: all clean