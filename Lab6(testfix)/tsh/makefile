INCS   = -I../include
OBJS   = ../obj 
FLAGS  = -g -w
cc     = gcc

.PHONY: all copy clean

all : tsh libtshlib.a launch tshtest copy 

tsh : tsh.c 
	$(cc) $(EXTRA) $(INCS) $(FLAGS) -o tsh tsh.c -L$(OBJS) -lsng

tshlib.o : tshlib.c
	$(cc) $(INCS) $(FLAGS) -c tshlib.c -o tshlib.o

libtshlib.a: tshlib.o
	ar rcs libtshlib.a tshlib.o


launch: launch.c libtshlib.a
	$(cc) $(INCS) $(FLAGS) -o launch launch.c -L$(OBJS) -lsng -L. -ltshlib

tshtest: tshtest.c libtshlib.a
	$(cc) $(INCS) $(FLAGS) -o tshtest tshtest.c -L$(OBJS) -lsng

copy: tsh tshlib launch
	cp tsh ../bin
	cp libtshlib.a ../bin
	cp launch ../bin
	cp tshtest ../bin

clean:
	rm -f tsh tshlib.o libtshlib.a launch tshtest