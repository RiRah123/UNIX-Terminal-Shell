CC=gcc
CFLAGS=-g -Wall
OLIBSFLAeS=-lreadline -lhistory 
LIBFLAGS=$(OLIBSFLAGS) -ltermcap -lcurses
CFLAGSO=-g -Wno-parentheses -Wno-format-security

#INC=-I/home/myid/ingrid/usr/local/include
#LIBLOC=/home/myid/ingrid/usr/local/lib

all: 	rlbasic 	\
	histexamp 	\
	shell 

%.o : %.c
	$(CC) $(CFLAGSO) $(DEF) $(INC) -c $<

rlbasic : rlbasic.o
	$(CC) $(CFLAGS) -o $@ $< $(LIBFLAGS)

histexamp : histexamp.o
	$(CC) $(CFLAGS) -o $@ $< $(LIBFLAGS)

shell:	shell.o parse.o parse.h
	$(CC) $(CFLAGS) -o $@ shell.o parse.o $(LIBFLAGS)

clean:
	rm -f shell *~ 
	rm -f pipe
	rm -f *.o
	rm -f rlbasic rlbasic.o
	rm -f histexamp histexamp.o
