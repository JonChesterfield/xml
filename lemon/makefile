
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

all::	main

clean::
	rm -f lemon main makeheaders
	rm -f *.o
	rm -f lemon parse.c parse.h parse.out
