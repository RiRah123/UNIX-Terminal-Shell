CC=gcc
CFLAGS=-g -Wall
LIBFLAGS=-ltermcap -lcurses
CFLAGSO=-g -Wno-parentheses -Wno-format-security

DEF=-DHAVE_CONFIG_H -DRL_LIBRARY_VERSION='"8.1"'
INC=-I/home/myid/ingrid/usr/local/include
LIBLOC=/home/myid/ingrid/usr/local/lib

all: 	rlbasic 	\
	histexamp 	\
	yosh

%.o : %.c
	$(CC) $(CFLAGSO) $(DEF) $(INC) -c $<

rlbasic : rlbasic.o
	$(CC) $(CFLAGS) -o $@ $< $(LIBLOC)/libreadline.a $(LIBFLAGS)

histexamp : histexamp.o
	$(CC) $(CFLAGS) -o $@ $< $(LIBLOC)/libhistory.a $(LIBFLAGS)

yosh:	yosh.o parse.o parse.h
	$(CC) $(CFLAGS) -o $@ yosh.o parse.o $(LIBLOC)/libreadline.a $(LIBFLAGS)

clean:
	rm -f yosh *~
	rm -f pipe
	rm -f *.o
	rm -f rlbasic rlbasic.o
	rm -f histexamp histexamp.o
