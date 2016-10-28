include ../Make.defines

PROGS =	rip 

all:	${PROGS}

rip:	rip.o
		${CC} ${CFLAGS} -o $@ rip.o ${LIBS}
clean:
		rm -f ${PROGS} ${CLEANFILES}

