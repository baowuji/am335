include ./Make.defines.linux
EXTRA=
EXTRALIBS=-pthread

PROGS = ctl_daemon can 

all:	${PROGS}

clean:
	rm -f ${PROGS} ${TEMPFILES} *.o

