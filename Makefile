CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g3

Bash:	mainBash.o process.o stack.o tokenize.o parse.o
	${CC} ${CFLAGS} -o $@ $^

process.o:	process.c
	${CC} ${CFLAGS} -c $<

clean: 
	rm -rf *.o