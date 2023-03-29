SRC := $(wildcard  *.l)                                                                               
EXE :=$(patsubst %.l, %, $(SRC))                                                                      
                                                                                                      
.phony: all clean                                                                                     
                                                                                                      
all: $(EXE)                                                                                           
                                                                                                      
%: %.l                                                                                                
	@flex -o $@.yy.c $<                                                                           
	@gcc $@.yy.c -o $@ -lfl                                                                       
                                                                                                      
clean:                                                                                                
	@rm -v $(EXE)