$(if $(regex_tmp),,$(error makefile requires regex_tmp))
$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

$(regex_tmp):
	@mkdir -p $@

clean::
	rm -rf $(regex_tmp)


REGEX_HEADERS := regex/regex.h regex/regex.ptree.h regex/regex.declarations.h tools/ptree.h tools/ptree_impl.h regex/regex.byte_constructors.data regex/regex.ptree.byte_print_array.data regex/regex.lexer.h regex/regex.production.h regex/regex_parser.lemon.h regex/regex_string.h

REGEX_SOURCE := regex.ptree.c regex.c regex.lexer.c regex_parser.lemon.c regex_parser.bison.c regex_string.c regex_driver.c regex_cache.c regex_equality.c regex_queries.c regex_interpreter.c


REGEX_PROGRAM_SOURCE := regex.tests.c regex_stdin_to_xml.c

REGEX_OBJECTS := $(addprefix $(regex_tmp)/,$(REGEX_SOURCE:.c=.o))
REGEX_PROGRAM_OBJECTS := $(addprefix $(regex_tmp)/,$(REGEX_PROGRAM_SOURCE:.c=.o))



REGEX_TOOLS_OBJECTS := $(addprefix $(TOOLS_DIR_OBJ)/,lexer.re2c.o lexer.posix.o lexer.re2.o stringtable.o intset.o intmap.o)

# going to patch lemon to allow specifying filenames I think

regex_lemon_tmp := $(regex_tmp)/lemon
$(regex_lemon_tmp):	$(regex_tmp)
	@mkdir -p $(regex_lemon_tmp)

$(regex_lemon_tmp)/%.lemon.c:	regex/%.lemon.y $(lemon) tools/lempar.data regex/regex.lang.xml | $(regex_lemon_tmp)
	cp "$<" "$(regex_lemon_tmp)/$*.lemon.y"
	cd $(regex_lemon_tmp) && ./../../$(lemon) -l -T../../tools/lempar.data -m "$*.lemon.y" || rm -f $@

$(regex_lemon_tmp)/%.lemon.h:	$(regex_lemon_tmp)/%.lemon.c $(makeheaders) | $(regex_lemon_tmp)
	cd $(regex_lemon_tmp)/ && ./../../$(makeheaders) "$*.lemon.c"

regex/regex_parser.lemon.c:	$(regex_lemon_tmp)/regex_parser.lemon.c
	@cp "$<" "$@"
#	ugly, but does hack around the unused variable warnings
	@sed -i 's~\(#define regex_LemonCTX_FETCH ptree_context regex_ptree_context=yypParser->regex_ptree_context;\)~\0 (void)regex_ptree_context;~g' "$@"

regex/regex_parser.lemon.h:	$(regex_lemon_tmp)/regex_parser.lemon.h
	@cp "$<" "$@"

regex/regex.ptree.h:	regex/regex.ptree.h.in tools/ptree_macro_wrapper.h
	$(CC) -E -C -P -xc $< -ffreestanding -o $@
	clang-format -i $@

clean::
	@rm -f regex/regex_parser.lemon.c regex/regex_parser.lemon.h
	@rm -f regex/regex.ptree.h



$(regex_tmp)/regex_lexer_declarations.TokenTree.xml:	lang_to_lexer_declarations_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

$(regex_tmp)/regex_lexer_definitions.TokenTree.xml:	lang_to_lexer_definitions_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

$(regex_tmp)/regex_lexer_re2c_iterator.TokenTree.xml:	lang_to_lexer_re2c_iterator_definition_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

$(regex_tmp)/regex.lexer_re2c_iterator.c.re2c:	$(regex_tmp)/regex_lexer_re2c_iterator.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"

$(regex_tmp)/regex_production_declarations.TokenTree.xml:	lang_to_production_declarations_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

$(regex_tmp)/regex_parser.lemon.TokenTree.xml:	lang_to_parser_lemon_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

$(regex_tmp)/regex_parser.bison.TokenTree.xml:	lang_to_parser_bison_TokenTree.xsl regex/regex.lang.xml  | $(regex_tmp)
	@xsltproc $(XSLTPROCOPTS) --output $@ $^

regex/regex.lexer.h:	$(regex_tmp)/regex_lexer_declarations.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"

# Put this next to regex.lexer.c, is #included by it
regex/regex_lexer_re2c_iterator_step.data:	$(regex_tmp)/regex.lexer_re2c_iterator.c.re2c
	re2c $^ --output $@ $(RE2COPTS)

regex/regex.lexer.c:	$(regex_tmp)/regex_lexer_definitions.hex regex/regex_lexer_re2c_iterator_step.data $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"


regex/regex.production.h:	$(regex_tmp)/regex_production_declarations.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"

regex/regex_parser.lemon.y:	$(regex_tmp)/regex_parser.lemon.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"

regex/regex_parser.bison.y:	$(regex_tmp)/regex_parser.bison.hex $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"

# multiple output files are messy in make
regex/regex_parser.bison.c regex/regex_parser.bison.h:	regex/regex_parser.bison.y
	bison --no-lines --header=regex/regex_parser.bison.h $^ -o regex/regex_parser.bison.c
regex/regex_parser.bison.h: regex/regex_parser.bison.c

clean::
	@rm -f regex/regex.lexer.h
	@rm -f regex/regex_lexer_re2c_iterator_step.data
	@rm -f regex/regex.lexer.c
	@rm -f regex/regex.production.h
	@rm -f regex/regex_parser.lemon.y

	@rm -f regex/regex_parser.bison.y
	@rm -f regex/regex_parser.bison.c
	@rm -f regex/regex_parser.bison.h
	@rm -f regex/regex_parser.bison.output

$(REGEX_PROGRAM_OBJECTS): regex/regex_parser.lemon.t
$(REGEX_OBJECTS) $(REGEX_PROGRAM_OBJECTS): $(regex_tmp)/%.o: regex/%.c $(REGEX_HEADERS) | $(regex_tmp) 
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
regex:	$(REGEX_OBJECTS) regex/regex_parser.lemon.c regex/regex_parser.lemon.t bin/regex.tests bin/regex_stdin_to_xml regex/regex_parser.bison.y

bin/regex.tests:	$(regex_tmp)/regex.tests.o $(REGEX_OBJECTS) $(REGEX_TOOLS_OBJECTS)
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $^ -o $@

bin/regex_stdin_to_xml:	$(regex_tmp)/regex_stdin_to_xml.o $(REGEX_OBJECTS) $(REGEX_TOOLS_OBJECTS)
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $^ -o $@

clean::
	@rm -f bin/regex.tests bin/regex_stdin_to_xml

regex/regex.lang.xml:	regex/regex.lang.xml.lua
	@lua $^ > $@

regex/regex.declarations.h:	regex/regex.declarations.h.lua
	@lua $^ > $@

regex/regex.byte_constructors.data:	regex/regex.byte_constructors.data.lua
	@lua $^ > $@

regex/regex.ptree.byte_print_array.data:	regex/regex.ptree.byte_print_array.data.lua
	@lua $^ > $@

regex/regex_stdin_to_xml.c:	scripts/write_stdin_to_xml.lua
	@lua $^ "regex" > $@

clean::
	@rm -f regex/regex.lang.xml
	@rm -f regex/regex.declarations.h
	@rm -f regex/regex.byte_constructors.data
	@rm -f regex/regex.ptree.byte_print_array.data
	@rm -f regex/regex_stdin_to_xml.c

