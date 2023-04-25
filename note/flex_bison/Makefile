FLEX_SRC := $(wildcard  *.l)                                                                               
BISON_SRC := $(wildcard *.y)
EXE :=$(patsubst %.l, %, $(FLEX_SRC))                                                                      

BUILD_DIR := build
                                                                                                      
.phony: all clean                                                                                     
                                                                                                      
all: $(BUILD_DIR) $(EXE)                                                                                           
                                                                                                      
%: %.l %.y                                                                                               
	flex -o $(BUILD_DIR)/$*.yy.c $*.l
	bison --header=$(BUILD_DIR)/$*.tab.h --output=$(BUILD_DIR)/$*.tab.c  $*.y
	@gcc $(BUILD_DIR)/$@.yy.c $(BUILD_DIR)/$*.tab.c -o $(BUILD_DIR)/$@                                                                    

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:                                                                                                
	@rm -rv $(BUILD_DIR)