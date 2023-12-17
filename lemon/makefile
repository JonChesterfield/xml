
all::
clean::

makeheaders:	makeheaders.c
	gcc $< -o $@

lemon:	lemon.c
	gcc $< -o $@

# -m produces a .c that makeheaders understands and
# suppresses the .h file creation
parse.c:	parse.y lempar.c lemon
	./lemon -Tlempar.c -m $<

parse.h:	parse.c makeheaders
	./makeheaders $<

parse.o:	parse.c parse.h
	gcc -c $< -o $@

main.o:	main.c
	gcc -c $< -o $@

main:	parse.o main.o
	gcc $^ -o $@


retest.o:	retest.cpp
	g++ -c $< -o $@

# apt install libre2-dev
#include <re2/re2.h>
retest:	retest.o
	g++ $^ /usr/lib/x86_64-linux-gnu/libre2.a -o $@

all::	main retest

clean::
	rm -f lemon main makeheaders retest
	rm -f *.o
	rm -f lemon parse.c parse.h parse.out
