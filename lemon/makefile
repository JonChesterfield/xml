
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
	g++ -std=c++14 -c $< -o $@

lexer.o:	lexer.cpp lexer.h
	g++ -std=c++14 -c $< -o $@

re2_lexer.o:	re2_lexer.cpp lexer.h parse.h types.h
	g++ -std=c++14 -c $< -o $@


re2cdemo.cpp:	re2cdemo.re
	re2c $< -o $@

re2cdemo.o:	re2cdemo.cpp
	g++ -std=c++14 -c $^ -o $@

re2cdemo:	re2cdemo.o
	g++ -std=c++14 $^ -o $@


arith.flex.xml:	lang_to_flex.xsl arith.lang.xml
	xsltproc --output $@ $^

arith.flex.cpp: arith.flex.xml parse.h
	xsltproc --output $@ ../subtransforms/drop_outer_element.xsl $<
	clang-format -i $@

# apt install libre2-dev
#include <re2/re2.h>
retest:	retest.o
	g++ -std=c++14 $^ /usr/lib/x86_64-linux-gnu/libre2.a -o $@

re2_lexer:	re2_lexer.o lexer.o parse.o
	g++ -std=c++14 $^ /usr/lib/x86_64-linux-gnu/libre2.a -o $@


all::	main retest re2cdemo re2_lexer

clean::
	rm -f lemon main makeheaders retest re2cdemo re2_lexer
	rm -f *.o
	rm -f lemon parse.c parse.h parse.out
	rm -f re2cdemo.cpp arith.flex.cpp
