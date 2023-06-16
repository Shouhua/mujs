SRC := $(wildcard *.c) $(wildcard *.cpp)
DST := $(patsubst %.c, %, $(SRC))
DST := $(patsubst %.cpp, %, $(DST))

all: $(DST)

%: %.c
	gcc -g -Wall $< -o $@ -lssl -lcrypto -DUSE_TLS13
%: %.cpp
	g++ -g -Wall $< -o $@ -lssl -lcrypto