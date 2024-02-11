$(if $(arith_tmp),,$(error makefile requires arith_tmp))
$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

clean::
	rm -rf $(arith_tmp)

# well this is grim, lemon is a bit too filename driven
# use a directory ad cd into it before calling the tools to control the output filename

lemon_tmp := $(arith_tmp)/lemon
$(lemon_tmp):	$(arith_tmp)
	@mkdir -p $(lemon_tmp)

$(lemon_tmp)/%.lemon.c:	arith/%.lemon.y $(lemon) tools/lempar.data arith/arith.lang.xml | $(lemon_tmp)
	cp "$<" "$(lemon_tmp)/$*.lemon.y"
	cd $(lemon_tmp) && ./../../$(lemon) -l -T../../tools/lempar.data -m "$*.lemon.y" || rm -f $@

$(lemon_tmp)/%.lemon.h:	$(lemon_tmp)/%.lemon.c $(makeheaders) | $(lemon_tmp)
	cd $(lemon_tmp)/ && ./../../$(makeheaders) "$*.lemon.c"

arith/arith.lemon.c:	$(lemon_tmp)/arith.lemon.c
	@cp "$<" "$@"

arith/arith.lemon.h:	$(lemon_tmp)/arith.lemon.h
	@cp "$<" "$@"

arith/arith_parser.lemon.c:	$(lemon_tmp)/arith_parser.lemon.c
	@cp "$<" "$@"

arith/arith_parser.lemon.h:	$(lemon_tmp)/arith_parser.lemon.h
	@cp "$<" "$@"

arith/arith.ptree.h:	arith/arith.ptree.h.in tools/ptree_macro_wrapper.h
	$(CC) -E -C -P -xc $< -ffreestanding -o $@
	clang-format -i $@


$(arith_tmp)/stdin_to_tree.o:	arith/stdin_to_tree.c arith/arith.lemon.h arith/arith.lexer.h arith/arith.declarations.h | $(arith_tmp)
	$(CC) $(CFLAGS) -c $< -o $@


clean::
	rm -f arith/arith.ptree.h
	rm -f arith/arith.lemon.h arith/arith.lemon.c
	rm -f arith/arith_parser.lemon.h arith/arith_parser.lemon.c

$(arith_tmp)/arith.ptree.o:	arith/arith.ptree.c arith/arith.ptree.h tools/ptree.h arith/arith.declarations.h | $(arith_tmp)
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@

$(arith_tmp)/arith.tests.o:	arith/arith.tests.c arith/arith.ptree.h tools/ptree.h arith/arith.declarations.h | $(arith_tmp)
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@

$(arith_tmp)/arith.lemon.o:	arith/arith.lemon.c arith/arith.lemon.h
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@

$(arith_tmp)/arith.main.o:	arith/arith.main.c arith/arith.lang.xml $(arith_tmp)
	$(CC) $(CFLAGS) -c $< -o $@

$(arith_tmp)/arith.lexer.xml:	lang_to_lexer.xsl arith/arith.lang.xml | $(arith_tmp)
	xsltproc --output $@ $^

$(arith_tmp)/arith_declarations.TokenTree.xml:	lang_to_declarations_TokenTree.xsl arith/arith.lang.xml  | $(arith_tmp)
	xsltproc --output $@ $^

$(arith_tmp)/arith_definitions.TokenTree.xml:	lang_to_definitions_TokenTree.xsl arith/arith.lang.xml  | $(arith_tmp)
	xsltproc --output $@ $^

$(arith_tmp)/arith_re2c_iterator.TokenTree.xml:	lang_to_lexer_re2c_iterator_definition_TokenTree.xsl arith/arith.lang.xml  | $(arith_tmp)
	xsltproc --output $@ $^


$(arith_tmp)/arith_parser.lemon.TokenTree.xml:	lang_to_parser_lemon_TokenTree.xsl arith/arith.lang.xml  | $(arith_tmp)
	xsltproc --output $@ $^

# TokenList is a common form
$(eval $(call XML_Pipeline_Template_Common,$(arith_tmp),TokenTree,TokenList))
$(eval $(call XML_Pipeline_Template_Common,$(arith_tmp),TokenList,HexTokenList))
$(eval $(call XML_Pipeline_Template_Common,$(arith_tmp),HexTokenList,RawBinary))

# This might be reasonable as a top level transform
# actually, so might the %.TokenTree.xml ->  %.TokenList.xml style ones
$(arith_tmp)/%.hex:	$(arith_tmp)/%.RawBinary.xml validate/subtransforms
	@xsltproc $(XSLTPROCOPTS) --output "$@" subtransforms/drop_outer_element.xsl "$<"

# TODO: ideally wouldn't depend on clang-format
arith/arith.declarations.h:	$(arith_tmp)/arith_declarations.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"
	clang-format -i $@


$(arith_tmp)/arith.re2c_iterator.c.re2c:	$(arith_tmp)/arith_re2c_iterator.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"
	clang-format -i $@

# Put this next to arith.definitions, is #included by it
arith/arith_lexer_re2c_iterator_step.data :	$(arith_tmp)/arith.re2c_iterator.c.re2c
	re2c $^ --output $@ $(RE2COPTS)
	clang-format -i $@

arith/arith.definitions.c:	$(arith_tmp)/arith_definitions.hex arith/arith_lexer_re2c_iterator_step.data $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"
	clang-format -i $@

arith/arith_parser.lemon.y:	$(arith_tmp)/arith_parser.lemon.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"

clean::
	rm -f arith/arith.declarations.h arith/arith.definitions.c arith/arith_lexer_re2c_iterator_step.data arith/arith_lexer_re2c.c arith/arith_parser.lemon.y

$(arith_tmp)/arith_lexer_re2c.o:	arith/arith_lexer_re2c.c
	$(CC) $(CFLAGS) -c $< -o $@

$(arith_tmp)/arith.definitions.o:	arith/arith.definitions.c arith/arith.declarations.h
	$(CC) $(CFLAGS) -c $< -o $@

arith/arith.lexer.cpp: $(arith_tmp)/arith.lexer.xml arith/arith.lemon.h arith/arith.declarations.h
	xsltproc --output $@ subtransforms/drop_outer_element.xsl $<
	clang-format -i $@

$(arith_tmp)/arith.lexer.o:	arith/arith.lexer.cpp tools/token.h tools/lexer_instance.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean::
	rm -f arith/arith.lexer.cpp
