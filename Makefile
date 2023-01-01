all: cim

cim: cim.c Makefile
	$(CC) cim.c -lcurses -o cim
