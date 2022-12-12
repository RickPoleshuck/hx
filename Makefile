# @(#) hx.mk - makefile for hx program 
CC=cc
CFLAGS=-M3 -O
CFLAGS=-g
LIBS=-lcurses
LDFLAGS=-s
OFILES=hx.o\
	hxcut.o\
	hxdisp.o\
	hxfline.o\
	hxgetfld.o\
	hxhard.o\
	hxpatch.o

all:	hx

$(OFILES):
	$(CC) -c $(CFLAGS) $*.c
hx:
	$(CC) $(LDFLAGS) -o hx $(OFILES) $(LIBS) 

hx:		$(OFILES)

clean:
	rm -f $(OFILES) hx

$(OFILES):		hx.h

