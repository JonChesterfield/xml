$(if $(regex_tmp),,$(error makefile requires regex_tmp))
$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

$(regex_tmp):
	@mkdir -p $@

clean::
	rm -rf $(regex_tmp)


REGEX_HEADERS := regex/regex.h regex/regex.ptree.h regex/regex.declarations.h tools/ptree.h tools/ptree_impl.h regex/regex.byte_constructors.data regex/regex.ptree.byte_print_array.data regex/regex.lexer.declarations.h regex/regex_parser.lemon.h regex/regex_string.h

REGEX_SOURCE := regex.ptree.c regex.tests.c regex.c regex.lexer.definitions.c regex_parser.lemon.c regex_string.c regex_driver.c regex_cache.c

REGEX_OBJECTS := $(addprefix $(regex_tmp)/,$(REGEX_SOURCE:.c=.o))


# going to patch lemon to allow specifying filenames I think

lemon_tmp := $(regex_tmp)/lemon
$(lemon_tmp):	$(regex_tmp)
	@mkdir -p $(lemon_tmp)

$(lemon_tmp)/%.lemon.c:	regex/%.lemon.y $(lemon) tools/lempar.data regex/regex.lang.xml | $(lemon_tmp)
	cp "$<" "$(lemon_tmp)/$*.lemon.y"
	cd $(lemon_tmp) && ./../../$(lemon) -l -T../../tools/lempar.data -m "$*.lemon.y" || rm -f $@

$(lemon_tmp)/%.lemon.h:	$(lemon_tmp)/%.lemon.c $(makeheaders) | $(lemon_tmp)
	cd $(lemon_tmp)/ && ./../../$(makeheaders) "$*.lemon.c"

regex/regex_parser.lemon.c:	$(lemon_tmp)/regex_parser.lemon.c
	@cp "$<" "$@"
#	ugly, but does hack around the unused variable warnings
	@sed -i 's~\(#define regex_LemonCTX_FETCH ptree_context regex_ptree_context=yypParser->regex_ptree_context;\)~\0 (void)regex_ptree_context;~g' "$@"

regex/regex_parser.lemon.h:	$(lemon_tmp)/regex_parser.lemon.h
	@cp "$<" "$@"



regex/regex.ptree.h:	regex/regex.ptree.h.in tools/ptree_macro_wrapper.h
	$(CC) -E -C -P -xc $< -ffreestanding -o $@
	clang-format -i $@

clean::
	@rm -f regex/regex_parser.lemon.c regex/regex_parser.lemon.h
	@rm -f regex/regex.ptree.h



$(regex_tmp)/regex_declarations.TokenTree.xml:	lang_to_declarations_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

$(regex_tmp)/regex_definitions.TokenTree.xml:	lang_to_definitions_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

$(regex_tmp)/regex_re2c_iterator.TokenTree.xml:	lang_to_lexer_re2c_iterator_definition_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

$(regex_tmp)/regex.re2c_iterator.c.re2c:	$(regex_tmp)/regex_re2c_iterator.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"
	clang-format -i $@

$(regex_tmp)/regex_parser.lemon.TokenTree.xml:	lang_to_parser_lemon_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^


# TokenList is a common form
$(eval $(call XML_Pipeline_Template_Common,$(regex_tmp),TokenTree,TokenList))
$(eval $(call XML_Pipeline_Template_Common,$(regex_tmp),TokenList,HexTokenList))
$(eval $(call XML_Pipeline_Template_Common,$(regex_tmp),HexTokenList,RawBinary))

$(regex_tmp)/%.hex:	$(regex_tmp)/%.RawBinary.xml validate/subtransforms
	@xsltproc $(XSLTPROCOPTS) --output "$@" subtransforms/drop_outer_element.xsl "$<"

regex/regex.lexer.declarations.h:	$(regex_tmp)/regex_declarations.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"
	clang-format -i $@


# Put this next to regex.definitions, is #included by it
regex/regex_lexer_re2c_iterator_step.data:	$(regex_tmp)/regex.re2c_iterator.c.re2c
	re2c $^ --output $@ $(RE2COPTS)
	sed -i 's_#include "regex\.declarations\.h"_#include "regex.lexer.declarations.h"_g' "$@"
	clang-format -i $@

regex/regex.lexer.definitions.c:	$(regex_tmp)/regex_definitions.hex regex/regex_lexer_re2c_iterator_step.data $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"
	sed -i 's_#include "regex\.declarations\.h"_#include "regex.lexer.declarations.h"_g' "$@"
	clang-format -i $@


regex/regex_parser.lemon.y:	$(regex_tmp)/regex_parser.lemon.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"


clean::
	@rm -f regex/regex.lexer.declarations.h
	@rm -f regex/regex_lexer_re2c_iterator_step.data
	@rm -f regex/regex.lexer.definitions.c
	@rm -f regex/regex_parser.lemon.y



$(REGEX_OBJECTS): $(regex_tmp)/%.o: regex/%.c $(REGEX_HEADERS) | $(regex_tmp)
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@



$(regex_tmp)/parser_header_gen.c: | $(regex_tmp)
	@echo 'int regex_Lemon_parser_header(void);' > $@
	@echo 'int main(void) { return regex_Lemon_parser_header(); }' >> $@

$(regex_tmp)/parser_header_gen: $(regex_tmp)/parser_header_gen.c $(regex_tmp)/regex_parser.lemon.o
	$(CC) $(CFLAGS) $^ -o $@ -Wl,--unresolved-symbols=ignore-all -static

$(regex_tmp)/regex_string.o:	regex/regex_parser.lemon.t
regex/regex_parser.lemon.t:	$(regex_tmp)/parser_header_gen
	@./$< > $@

clean::
	@rm -f regex/regex_parser.lemon.t

.PHONY: regex
regex:	$(REGEX_OBJECTS) regex/regex_parser.lemon.c regex/regex_parser.lemon.t

bin/regex.tests:	$(REGEX_OBJECTS) .tools.O/lexer.re2c.o .tools.O/lexer.posix.o .tools.O/lexer.re2.o .tools.O/stringtable.o .tools.O/intset.o
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $^ -o $@

#bin/regex:	$(regex_tmp)/regex.o $(regex_tmp)/regex.ptree.o
#	@mkdir -p "$(dir $@)"
#	$(CC) $(CFLAGS) $^ -o $@


clean::
	@rm -f bin/regex.tests


regex/regex.declarations.h:	regex/regex.declarations.h.lua
	@lua $^ > $@

regex/regex.byte_constructors.data:	regex/regex.byte_constructors.data.lua
	@lua $^ > $@

regex/regex.ptree.byte_print_array.data:	regex/regex.ptree.byte_print_array.data.lua
	@lua $^ > $@

clean::
	@rm -f regex/regex.declarations.h regex/regex.byte_constructors.data regex/regex.ptree.byte_print_array.data

