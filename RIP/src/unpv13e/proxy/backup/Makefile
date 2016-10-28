include ../Make.defines

PROGS =	proxy_server 

all:	${PROGS}

proxy_server:	proxy_server.o
		${CC} ${CFLAGS} -o $@ proxy_server.o ${LIBS}
clean:
		rm -f ${PROGS} ${CLEANFILES}

