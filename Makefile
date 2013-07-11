include ./Make.defines.linux
EXTRA=
EXTRALIBS=-pthread

PROGS = ctl_daemon client

all:	${PROGS}

clean:
	rm -f ${PROGS} ${TEMPFILES} *.o

