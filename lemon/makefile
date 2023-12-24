
all::
clean::

CXXFLAGS := -std=c++14 -Wno-c99-designator

makeheaders:	makeheaders.c
	gcc $< -o $@

lemon:	lemon.c
	gcc $< -o $@

# -m produces a .c that makeheaders understands and
# suppresses the .h file creation
# returns non-zero on parsing conflicts, try to delete the file in that case
parse.c:	parse.y lempar.c lemon
	./lemon -Tlempar.c -m $< || rm -f $@

parse.h:	parse.c makeheaders
	./makeheaders $<

parse.o:	parse.c parse.h
	gcc -c $< -o $@

main.o:	main.c
	gcc -c $< -o $@

main:	parse.o main.o
	gcc $^ -o $@


retest.o:	retest.cpp
	g++ $(CXXFLAGS) -c $< -o $@

re2cdemo.cpp:	re2cdemo.re
	re2c $< -o $@

re2cdemo.o:	re2cdemo.cpp
	g++ $(CXXFLAGS) -c $^ -o $@

re2cdemo:	re2cdemo.o
	g++ $(CXXFLAGS) $^ -o $@




arith.lexer.xml:	lang_to_lexer.xsl arith.lang.xml
	xsltproc --output $@ $^

arith.lexer.cpp: arith.lexer.xml parse.h
	xsltproc --output $@ ../subtransforms/drop_outer_element.xsl $<
	clang-format -i $@

TokenList.lexer.xml:	lang_to_lexer.xsl TokenList.lang.xml
	xsltproc --output $@ $^
TokenList.lexer.cpp: TokenList.lexer.xml parse.h
	xsltproc --output $@ ../subtransforms/drop_outer_element.xsl $<
	clang-format -i $@
TokenList.lexer.o:	TokenList.lexer.cpp token.h lexer_instance.hpp
	g++ $(CXXFLAGS) -c $< -o $@
TokenList.lexer:	TokenList.lexer.o parse.o
	g++ $(CXXFLAGS) $^ /usr/lib/x86_64-linux-gnu/libre2.a -o $@


# apt install libre2-dev
#include <re2/re2.h>
retest:	retest.o
	g++ $(CXXFLAGS) $^ /usr/lib/x86_64-linux-gnu/libre2.a -o $@

arith.lexer.o:	arith.lexer.cpp token.h lexer_instance.hpp
	g++ $(CXXFLAGS) -c $< -o $@

arith.lexer:	arith.lexer.o parse.o
	g++ $(CXXFLAGS) $^ /usr/lib/x86_64-linux-gnu/libre2.a -o $@

TokenList.rng:	TokenList.rnc
	trang -Irnc -Orng $< $@

clean::
	rm -f TokenList.rng

arith.xml:	arith.lexer TokenList.rng
	echo "1 + 2 / 1 * (7 % 3)" | ./arith.lexer > $@
	xmllint --relaxng TokenList.rng $@

arith.txt:	TokenList_to_bytes.xsl arith.xml
	xsltproc --output $@ $^

all::	main retest re2cdemo arith.lexer arith.txt

clean::
	rm -f lemon main makeheaders retest re2cdemo arith.lexer
	rm -f *.o
	rm -f arith.lexer.xml arith.xml arith.txt
	rm -f lemon parse.c parse.h parse.out
	rm -f re2cdemo.cpp arith.lexer.cpp

# At the end to depend on the included makefiles as well as this one
.EXTRA_PREREQS+=$(foreach mk, ${MAKEFILE_LIST},$(abspath ${mk}))
