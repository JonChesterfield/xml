$(if $(regex_tmp),,$(error makefile requires regex_tmp))
$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

$(regex_tmp):
	@mkdir -p $@

clean::
	rm -rf $(regex_tmp)


# Currently lemon is implemented and bison most of the way there
# Considering also implementing byacc.
# A top level dispatch to run through all parsers and check they made the same
# conclusion has somewhat diminishing returns as N increases.

REGEX_HEADERS := regex/regex.h regex/regex.ptree.h regex/regex.declarations.h tools/ptree.h tools/ptree_impl.h regex/regex.ptree.byte_print_array.data regex/regex.lexer.h   regex/regex_string.h regex/ascii.declarations.h regex/regex_grouping_byte_cases.data

REGEX_HEADERS := $(REGEX_HEADERS) regex/regex.parser_bison.h regex/regex.parser_lemon.h
REGEX_HEADERS := $(REGEX_HEADERS) regex/regex.lexer.h regex/regex.productions.h
REGEX_HEADERS := $(REGEX_HEADERS) regex/regex.lexer_re2c_iterator.data

REGEX_HEADERS := $(REGEX_HEADERS) regex/regex_interpreter.h regex/ascii_interpreter.h regex/interpreter.data

REGEX_SOURCE := regex.ptree.c regex.c regex.lexer.c regex_string.c regex_driver.c regex_cache.c regex_equality.c regex_queries.c ascii_regex_consistent.c

REGEX_SOURCE := $(REGEX_SOURCE) regex.parser_bison.c regex.parser_lemon.c
REGEX_SOURCE := $(REGEX_SOURCE) regex.production_assign.c regex.production_custom.c regex.production_list.c

REGEX_SOURCE := $(REGEX_SOURCE) regex_interpreter.c ascii_interpreter.c ascii.ptree.c

ASCII_HEADERS := ascii.lexer.h ascii.productions.h ascii.parser_bison.h ascii.parser_lemon.h ascii.lexer_re2c_iterator.data

REGEX_HEADERS := $(REGEX_HEADERS) $(addprefix regex/,$(ASCII_HEADERS))

REGEX_SOURCE := $(REGEX_SOURCE) ascii.lexer.c ascii.parser_bison.c ascii.parser_lemon.c
REGEX_SOURCE := $(REGEX_SOURCE) ascii.production_assign.c ascii.production_custom.c ascii.production_list.c
REGEX_SOURCE := $(REGEX_SOURCE) regex_match_tests.c

REGEX_PROGRAM_SOURCE := regex.tests.c regex_stdin_to_xml.c ascii_stdin_to_xml.c

REGEX_OBJECTS := $(addprefix $(regex_tmp)/,$(REGEX_SOURCE:.c=.o))
REGEX_PROGRAM_OBJECTS := $(addprefix $(regex_tmp)/,$(REGEX_PROGRAM_SOURCE:.c=.o))


REGEX_TOOLS_OBJECTS := $(addprefix $(TOOLS_DIR_OBJ)/,lexer.re2c.o lexer.posix.o lexer.re2.o stringtable.o intset.o intstack.o intmap.o)


# Lemon either overwrites the .h file, or if passed -m includes its guess at where
# the mkheaders result would be, neither of which is helpful behaviour here
# Leave off -m so it doesn't introduce an include of a header from the wrong directory
# Write into temporary dir so it doesn't clobber the header


regex/regex.ptree.h:	regex/regex.ptree.h.in tools/ptree_macro_wrapper.h
	$(CC) -E -C -P -xc $< -ffreestanding -o $@
	clang-format -i $@

clean::
	@rm -f regex/regex.ptree.h

$(REGEX_PROGRAM_OBJECTS): regex/regex.parser_lemon.t regex/ascii.parser_lemon.t
$(REGEX_OBJECTS) $(REGEX_PROGRAM_OBJECTS): $(regex_tmp)/%.o: regex/%.c $(REGEX_HEADERS) | $(regex_tmp) 
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@

$(regex_tmp)/regex_parser_lemon_header_gen.c: | $(regex_tmp)
	@echo 'int regex_parser_lemon_type_header(void);' > $@
	@echo 'int main(void) { return regex_parser_lemon_type_header(); }' >> $@

$(regex_tmp)/regex_parser_bison_header_gen.c: | $(regex_tmp)
	@echo 'int regex_parser_bison_type_header(void);' > $@
	@echo 'int main(void) { return regex_parser_bison_type_header(); }' >> $@

$(regex_tmp)/regex_parser_lemon_header_gen: $(regex_tmp)/regex_parser_lemon_header_gen.c $(regex_tmp)/regex.parser_lemon.o
	$(CC) $(CFLAGS) $^ -o $@ -Wl,--unresolved-symbols=ignore-all -static

$(regex_tmp)/regex_parser_bison_header_gen: $(regex_tmp)/regex_parser_bison_header_gen.c $(regex_tmp)/regex.parser_bison.o
	$(CC) $(CFLAGS) $^ -o $@ -Wl,--unresolved-symbols=ignore-all -static

$(regex_tmp)/ascii_parser_lemon_header_gen.c: | $(regex_tmp)
	@echo 'int ascii_parser_lemon_type_header(void);' > $@
	@echo 'int main(void) { return ascii_parser_lemon_type_header(); }' >> $@

$(regex_tmp)/ascii_parser_bison_header_gen.c: | $(regex_tmp)
	@echo 'int ascii_parser_bison_type_header(void);' > $@
	@echo 'int main(void) { return ascii_parser_bison_type_header(); }' >> $@

$(regex_tmp)/ascii_parser_lemon_header_gen: $(regex_tmp)/ascii_parser_lemon_header_gen.c $(regex_tmp)/ascii.parser_lemon.o
	$(CC) $(CFLAGS) $^ -o $@ -Wl,--unresolved-symbols=ignore-all -static

$(regex_tmp)/ascii_parser_bison_header_gen: $(regex_tmp)/ascii_parser_bison_header_gen.c $(regex_tmp)/ascii.parser_bison.o
	$(CC) $(CFLAGS) $^ -o $@ -Wl,--unresolved-symbols=ignore-all -static


$(regex_tmp)/regex_string.o:	regex/regex.parser_lemon.t

regex/regex.parser_lemon.t:	$(regex_tmp)/regex_parser_lemon_header_gen
	@./$< > $@
regex/regex.parser_bison.t:	$(regex_tmp)/regex_parser_bison_header_gen
	@./$< > $@

regex/ascii.parser_lemon.t:	$(regex_tmp)/ascii_parser_lemon_header_gen
	@./$< > $@
regex/ascii.parser_bison.t:	$(regex_tmp)/ascii_parser_bison_header_gen
	@./$< > $@


clean::
	@rm -f regex/regex.parser_lemon.t
	@rm -f regex/ascii.parser_lemon.t
	@rm -f regex/regex.parser_bison.t
	@rm -f regex/ascii.parser_bison.t

.PHONY: regex

regex::	$(REGEX_OBJECTS) regex/regex.parser_lemon.t regex/regex.parser_bison.t bin/regex.tests bin/regex_stdin_to_xml bin/ascii_stdin_to_xml

bin/regex.tests:	$(regex_tmp)/regex.tests.o $(REGEX_OBJECTS) $(REGEX_TOOLS_OBJECTS)
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $^ -o $@

bin/regex_stdin_to_xml:	$(regex_tmp)/regex_stdin_to_xml.o $(REGEX_OBJECTS) $(REGEX_TOOLS_OBJECTS)
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $^ -o $@

bin/ascii_stdin_to_xml:	$(regex_tmp)/ascii_stdin_to_xml.o $(REGEX_OBJECTS) $(REGEX_TOOLS_OBJECTS)
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $^ -o $@

clean::
	@rm -f bin/regex.tests bin/regex_stdin_to_xml bin/ascii_stdin_to_xml

regex/regex.declarations.h:	regex/regex.declarations.h.lua
	@lua $^ > $@

regex/ascii.declarations.h:	regex/ascii.declarations.h.lua
	@lua $^ > $@

regex/regex.ptree.byte_print_array.data:	regex/regex.ptree.byte_print_array.data.lua
	@lua $^ > $@

regex/regex_grouping_byte_cases.data:	regex/regex_grouping_byte_cases.data.lua
	@lua $^ > $@

regex/regex_stdin_to_xml.c:	scripts/write_stdin_to_xml.lua
	@lua $^ "regex" > $@

regex/ascii_stdin_to_xml.c:	scripts/write_stdin_to_xml.lua
	@lua $^ "ascii" > $@

clean::
	@rm -f regex/regex.declarations.h regex/ascii.declarations.h
	@rm -f regex/regex.ptree.byte_print_array.data regex/regex_grouping_byte_cases.data
	@rm -f regex/regex_stdin_to_xml.c regex/ascii_stdin_to_xml.c

