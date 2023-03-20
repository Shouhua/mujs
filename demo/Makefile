BUILD_DIR := build
SRC := $(wildcard *.c)
EXE := $(patsubst %.c, $(BUILD_DIR)/%, $(SRC))

all: $(BUILD_DIR) $(EXE)

$(BUILD_DIR)/%: %.c
	gcc $< ../build/debug/libmujs.o -g -Wall -pedantic -o ./$@ -lm -I..
$(BUILD_DIR):
	mkdir -p build

clean:
	rm -rf $(BUILD_DIR)/*
