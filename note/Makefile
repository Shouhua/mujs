BUILD_DIR := build
SRC := $(wildcard *.c)
EXE := $(patsubst %.c, $(BUILD_DIR)/%, $(SRC))
CFLAGS := -Wall -pedantic -g
CLIBS := -lm -lcurl -levent
CINCLUDES := -I../..
MUJS_OBJ := ../../build/debug/libmujs.o

all: $(BUILD_DIR) $(EXE)

$(BUILD_DIR)/%: %.c
	gcc $< $(MUJS_OBJ) $(CFLAGS) -o ./$@ $(CLIBS) $(CINCLUDES)
$(BUILD_DIR):
	mkdir -p build

clean:
	rm -rf $(BUILD_DIR)/*
