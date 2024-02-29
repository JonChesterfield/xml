$(if $(regex_tmp),,$(error makefile requires regex_tmp))
$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

$(regex_tmp):
	@mkdir -p $@

clean::
	rm -rf $(regex_tmp)


regex/regex.ptree.h:	regex/regex.ptree.h.in tools/ptree_macro_wrapper.h
	$(CC) -E -C -P -xc $< -ffreestanding -o $@
	clang-format -i $@

$(regex_tmp)/regex.ptree.o:	regex/regex.ptree.c regex/regex.ptree.h tools/ptree.h tools/ptree_impl.h regex/regex.declarations.h regex/regex.ptree.byte_print_array.data | $(regex_tmp)
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@

$(regex_tmp)/regex.tests.o:	regex/regex.tests.c regex/regex.ptree.h tools/ptree.h tools/ptree_impl.h regex/regex.declarations.h | $(regex_tmp)
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@

$(regex_tmp)/regex.o:	regex/regex.c regex/regex.ptree.h tools/ptree.h tools/ptree_impl.h regex/regex.declarations.h | $(regex_tmp)
	$(CC) $(CFLAGS) -Wno-unused-parameter -c $< -o $@


bin/regex.tests:	$(regex_tmp)/regex.tests.o $(regex_tmp)/regex.ptree.o
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $^ -o $@

bin/regex:	$(regex_tmp)/regex.o $(regex_tmp)/regex.ptree.o
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $^ -o $@


regex/regex.declarations.h:	regex/regex.declarations.h.lua
	@lua $^ > $@

regex/regex.ptree.byte_print_array.data:	regex/regex.ptree.byte_print_array.data.lua
	@lua $^ > $@

clean::
	@rm -f regex/regex.ptree.h regex/regex.declarations.h
	@rm -f bin/regex.tests
	@rm -f regex/regex.ptree.byte_print_array.data
