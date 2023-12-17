
all::
clean::

lemon:	lemon.c
	gcc $< -o $@

parse.c parse.h:	parse.y lempar.c lemon
	./lemon -Tlempar.c $<

parse.o:	parse.c
	gcc -c $< -o $@

main.o:	main.c
	gcc -c $< -o $@

main:	parse.o main.o
	gcc $^ -o $@

all::	main

clean::
	rm -f lemon main
	rm -f *.o
	rm -f lemon parse.c parse.h parse.out
