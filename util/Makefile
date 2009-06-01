CFLAGS=-O
LDLIBS=-lz
all: yeposc deyepos dict2y ysort
clean:
	rm yeposc.o deyepos.o dict2y.o ysort.o yeposc deyepos dict2y ysort
yeposc.o:../include/signs.h
deyepos.o:../include/signs.h
dict2y.o:dynamic_buffer.h
ysort.o:dynamic_buffer.h
.SUFFIXES: .c .o
.PHONY: all clean
yeposc: yeposc.o ; ${CC} ${LDFLAGS} -o $@ yeposc.o ${LDLIBS}
deyepos: deyepos.o ; ${CC} ${LDFLAGS} -o $@ deyepos.o ${LDLIBS}
dict2y: dict2y.o ; ${CC} ${LDFLAGS} -o $@ dict2y.o
ysort: ysort.o ; ${CC} ${LDFLAGS} -o $@ ysort.o
