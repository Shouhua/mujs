SRC := $(wildcard *.c)
TARGET := $(patsubst %.c, build/%, $(SRC))

all: build $(TARGET)

build:
	mkdir -p build

build/%: %.c
	gcc -g -Wall $< -o $@ -lssl -lcrypto -DUSE_TLS13