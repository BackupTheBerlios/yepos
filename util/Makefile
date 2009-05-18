.PHONY: all clean
CFLAGS=-Wall -O2
LDLIBS=-lz
all: yeposc deyepos dict2y ysort
clean:
	rm yeposc.o deyepos.o dict2y.o ysort.o yeposc deyepos dict2y ysort
yeposc.o:../include/signs.h
deyepos.o:../include/signs.h
dict2y.o:dynamic_buffer.h
ysort.o:dynamic_buffer.h
