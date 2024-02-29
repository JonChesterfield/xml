$(if $(regex_tmp),,$(error makefile requires regex_tmp))
$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

$(regex_tmp):
	@mkdir -p $@

clean::
	rm -rf $(regex_tmp)


REGEX_HEADERS := regex/regex.h regex/regex.ptree.h regex/regex.declarations.h tools/ptree.h tools/ptree_impl.h regex/regex.byte_constructors.data regex/regex.ptree.byte_print_array.data regex/regex.lexer.declarations.h

REGEX_SOURCE := regex.ptree.c regex.tests.c regex.c regex.lexer.definitions.c

regex/regex.ptree.h:	regex/regex.ptree.h.in tools/ptree_macro_wrapper.h
	$(CC) -E -C -P -xc $< -ffreestanding -o $@
	clang-format -i $@

clean::
	@rm -f regex/regex.ptree.h



$(regex_tmp)/regex_declarations.TokenTree.xml:	lang_to_declarations_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	xsltproc --output $@ $^

$(regex_tmp)/regex_definitions.TokenTree.xml:	lang_to_definitions_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	xsltproc --output $@ $^

$(regex_tmp)/regex_re2c_iterator.TokenTree.xml:	lang_to_lexer_re2c_iterator_definition_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	xsltproc --output $@ $^

$(regex_tmp)/regex.re2c_iterator.c.re2c:	$(regex_tmp)/regex_re2c_iterator.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"
	clang-format -i $@



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


clean::
	@rm -f regex/regex.lexer.declarations.h
	@rm -f regex/regex_lexer_re2c_iterator_step.data
	@rm -f regex/regex.lexer.definitions.c


$(addprefix $(regex_tmp)/,$(REGEX_SOURCE:.c=.o)) : $(regex_tmp)/%.o: regex/%.c $(REGEX_HEADERS) | $(regex_tmp)
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@


bin/regex.tests:	$(regex_tmp)/regex.tests.o $(regex_tmp)/regex.o $(regex_tmp)/regex.ptree.o
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

