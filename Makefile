ARCH=arm
CROSS_COMPILE=arm-arago-linux-gnueabi-
WKDIR=.
CC=$(CROSS_COMPILE)gcc
COMPILE.c=$(CC) $(CFLAGS) $(CPPFLAGS) -c 
LINK.c=$(CC) $(CFLAGS) $(CPPFLAGS) $(LDDIR) $(LDFLAGS)
LDLIBS= $(EXTRALIBS)
CFLAGS=-DLINUX  -I$(WKDIR)/include -Wall -Wextra -g -D_GNU_SOURCE $(EXTRA)


TEMPFILES=core core.* *.o temp.* *.out typescript*

EXTRA= 
EXTRALIBS=-pthread

objects=ctl_daemon_util.o can.o arb.o aotf.o laser.o
PROGS = ctl_daemon 
all:ctl_daemon client

client: client.c
	gcc client.c -lm -o client	
ctl_daemon:$(objects) ctl_daemon.c
	$(CC) $(objects)  ctl_daemon.c -o ctl_daemon $(CFLAGS) $(LDLIBS) libsocketcan.a 
$(objectc):$(objects).c



clean:
	rm -f ${PROGS} ${TEMPFILES} *.o

