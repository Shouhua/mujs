SRC := $(wildcard *.c)
TARGET := $(patsubst %.c, build/%, $(SRC))

MUJS_OBJ := ../../build/debug/libmujs.o

all: build $(TARGET)

build:
	mkdir -p build

build/%: %.c
	gcc -g -Wall -pedantic -Wextra $< $(MUJS_OBJ) -o $@ -levent -lcurl -lm