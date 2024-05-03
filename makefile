# Fair warning, this is a dubious use of make.

# This makefile is used in an ad hoc framework sort of fashion to provide common
# infrastructure for projects held in subdirectories. E.g. /Planning or /regex
# are somewhat self-contained text munging style things that use code factored
# into this root level makefile.

# The original idea was to build a bunch of code generators on XSLT transforms
# Since then it has mutated somewhat into building code generators on C. This
# makefile therefore also compiles a bunch of C into libraries and executables
# that are used by the subprojects.

.SUFFIXES:
MAKEFLAGS += -r -j$(shell nproc)
.SECONDARY:
# .DELETE_ON_ERROR:

SHELL = sh -xv

# Design notes.
# Slowly moving towards a more framework layout. This file can define
# various transforms which are usable in subdirectories by referring
# to the right name - foo.bar from foo.baz assuming the files are to be
# adjacent to one another.
#
# The xml validation fits that pattern really, could pick a common
# suffix (.xml.prelint?) which is convertible to xml by passing validation
# e.g. TokenList is a known format, found under common (could be under schemas)
# %.TokenList.xml.prelint -> %.TokenList.xml involves xmllint followed by cp
# (possibly copy-if-changed, could break a dependency chain there)
#
# Likewise for conversions between known formats, e.g. TokenTree_to_TokenList.xsl
# exists, that could take a %.TokenTree.xml and build a %.TokenList.xml.prelint
# which then feeds into the previous.

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

.PHONY: all
all::

.PHONY: clean
clean::

# uses a global c, c++ compiler
# apt install trang xsltproc libxml2-utils

CC := clang
CXX := clang++

C_OR_CXX_FLAGS := -Wall -Wextra -Wcovered-switch-default -g -gdwarf-4
CFLAGS := -std=c11 $(C_OR_CXX_FLAGS) -O1
CXXFLAGS := -std=c++14 -Wno-c99-designator $(C_OR_CXX_FLAGS)
LDFLAGS := -lm

# Runs fine under statically linked musl
# CC=$(MAKEFILE_DIR)/musl/install/bin/clang
# CXX=$(MAKEFILE_DIR)/musl/install/bin/clang++
# LDFLAGS := -static

# Make is not presently invalidating everything when this file changes
TRIPLE := x86_64-unknown-linux-musl
#TRIPLE := aarch64-unknown-linux-musl
#TRIPLE := wasm32-unknown-wasi
CTARGET := --target=$(TRIPLE) --sysroot=$(MAKEFILE_DIR)/wasm/install/$(TRIPLE)


# wasi needs -mllvm -wasm-enable-sjlj for longjmp

CC=$(MAKEFILE_DIR)/wasm/install/bin/clang
CXX=$(MAKEFILE_DIR)/wasm/install/bin/clang++
CFLAGS := $(CTARGET) $(CFLAGS) 
CXXFLAGS := $(CTARGET) $(CXXFLAGS)
LDFLAGS := -static #-Wl,--sysroot=$(MAKEFILE_DIR)/wasm/install/$(TRIPLE)

# native doesn't use one, running aarch64 on x64 could use qemu,
# wasm needs one. In this case, built against x64 musl then copied to top level dir
# Error: missing imported function ('wasi_snapshot_preview1.path_filestat_get')
# Probably available via libuv
# Involves cloning
# https://github.com/vshymanskyy/uvwasi and
# https://github.com/libuv/libuv.git
# probably specific commits / branches of them, and splicing them into wasm3
# Maybe build libuv as a general dependency, lua uses the same lib
INTERPRETER := $(MAKEFILE_DIR)/wasm3

TARGET_CFLAGS :=



# Considering a single source single file approach to tools
file_to_cdata := $(INTERPRETER) bin/file_to_cdata
hex_to_binary := $(INTERPRETER) bin/hex_to_binary
lemon := $(INTERPRETER) bin/lemon
makeheaders := $(INTERPRETER) bin/makeheaders

# These aren't very single source but are still compilable
xmllint := $(INTERPRETER) bin/xmllint
xsltproc := $(INTERPRETER) bin/xsltproc

cmark := $(INTERPRETER) bin/cmark

# Source under TOOLS_DIR used to make binaries under TOOLS_DIR_BIN
TOOLS_DIR := tools
TOOLS_DIR_BIN := bin
TOOLS_DIR_OBJ := .$(TOOLS_DIR).O

include submakefiles/schemas.mk

# This doens't work as it should - dropdtd does not suppress the warning
# on missing dtd if also doing some other verification
# Patching around by patching cmark/xml.c to not write the DOCTYPE
XMLLINTOPTS := --nonet --huge --noout --dropdtd

# The xml pretty printer tends to trip the depth limit when it's the default 3000
# novalid stops it looking for .dtd files, notably CommonMark.dtd
# this is getting borderline, need to build xsltproc with O2 and push these limits up
XSLTPROCOPTS := --huge  --maxdepth 40000 --maxvars 150000 --novalid

RE2COPTS := --no-debug-info -W -Wno-useless-escape --no-generation-date

#
# Global transform rules. Make a file next to the current file with a different suffix.
#

# Any .md file to commonmark xml or html
%.cmark.xml:	%.md | $(cmark)
	@./$(cmark) --to xml $^ > $@

%.cmark.html:	%.md | $(cmark)
	@./$(cmark) --to html $^ > $@

# In any directory:
# %.TokenTree.xml -> %.TokenList.xml
# %.TokenList.xml -> %.HexTokenList.xml
# %.HexTokenList.xml -> %.RawBinary.xml
include $(SELF_DIR)common/common.mk

# When xsltproc output is empty it fails to create an empty output file
%.hex:	%.RawBinary.xml validate/subtransforms
	@touch "$@"
	$(xsltproc) $(XSLTPROCOPTS) --output "$@" subtransforms/drop_outer_element.xsl "$<"

# Not very pretty but doesn't matter much if bison is run twice
# better is probably to build the header directly from the xml
%.parser_bison.c:	%.parser_bison.y
	bison --no-lines --header=$*.parser_bison.h $^ --output=$@ -Dparse.trace

%.parser_bison.h:	%.parser_bison.y
	bison --no-lines --header=$*.parser_bison.h $^ --output=/dev/null -Dparse.trace


# Lemon writes a header file by default, the local version is hacked to not do that
%.parser_lemon.c:	%.parser_lemon.y | $(lemon)
	$(lemon) -l -Ttools/lempar.data $^ -d$(dir $^)

%.parser_lemon.h:	scripts/lemon_header.lua
	lua $^ "$(basename $(basename $(notdir $@)))" > $@

#
# make functions
#

rwildcard = $(foreach d,$(wildcard $(1)*),$(call rwildcard,$(d)/,$(2)) $(filter $(subst *,%,$(2)),$(d)))

# sequence of dir/name.lang.xml. Names need to be unique, don't need to match dir.
languages := regex/regex.lang.xml arith/arith.lang.xml regex/ascii.lang.xml langtest/test.lang.xml

lang_tmp := .lang.O
$(lang_tmp):
	@mkdir -p $@
clean::
	@rm -rf $(lang_tmp)


# Construct a TokenTree.xml from each lang/*.xsl generator
lang_transforms := lexer.h.TokenTree.xsl lexer.c.TokenTree.xsl
lang_transforms := $(lang_transforms) parser_lemon.y.TokenTree.xsl parser_bison.y.TokenTree.xsl
lang_transforms := $(lang_transforms) productions.h.TokenTree.xsl
lang_transforms := $(lang_transforms) production_assign.c.TokenTree.xsl production_list.c.TokenTree.xsl
lang_transforms := $(lang_transforms) lexer_re2c_iterator.data.re2c.TokenTree.xsl


define LANGTRANSFORMS
$(lang_tmp)/%.$(basename $1).xml:	lang/$1 $(lang_tmp)/%.lang.xml
	$(xsltproc) $(XSLTPROCOPTS) --output $$@ $$^
endef
$(foreach xform,$(lang_transforms),$(eval $(call LANGTRANSFORMS,$(xform))))
#$(foreach xform,$(lang_transforms),$(info $(call LANGTRANSFORMS,$(xform))))


# Add construction of these to lang::
generated_language_suffixes := parser_bison.c parser_bison.h
generated_language_suffixes := $(generated_language_suffixes) parser_lemon.c parser_lemon.h
generated_language_suffixes := $(generated_language_suffixes) lexer_re2c_iterator.data

clean_generated_language_suffixes := parser_lemon.out


# Copy the xml file into lang_tmp and the result back out
define LANGTEMPLATE
# $$(info "Language template for $1, create $(lang_tmp)/$1")

$(lang_tmp)/$(dir $1): $(lang_tmp)
	@mkdir -p "$$@"

# copy the lang xml file into tmp
$(lang_tmp)/$1:	$1 | $(xmllint) $(xsltproc) $(lang_tmp)/$(dir $1) validate/subtransforms
	@cp "$$<" "$$@"

# copy from temp directory into the source tree
# enumerating the suffixes helps avoid the patterns colliding with others
$(dir $1)$(basename $(basename $(notdir $1))).%.c: $(lang_tmp)/$(dir $1)$(basename $(basename $(notdir $1))).%.c
	@cp "$$<" "$$@"

$(dir $1)$(basename $(basename $(notdir $1))).%.h: $(lang_tmp)/$(dir $1)$(basename $(basename $(notdir $1))).%.h
	@cp "$$<" "$$@"

$(dir $1)$(basename $(basename $(notdir $1))).%.y: $(lang_tmp)/$(dir $1)$(basename $(basename $(notdir $1))).%.y
	@cp "$$<" "$$@"

$(dir $1)$(basename $(basename $(notdir $1))).%.data: $(lang_tmp)/$(dir $1)$(basename $(basename $(notdir $1))).%.data
	@cp "$$<" "$$@"

$(dir $1)$(basename $(basename $(notdir $1))).%.re2c: $(lang_tmp)/$(dir $1)$(basename $(basename $(notdir $1))).%.re2c
	@cp "$$<" "$$@"

$(basename $(basename $(notdir $1)))::	$(addprefix $(dir $1)$(basename $(basename $(notdir $1)))., $(lang_transforms:.TokenTree.xsl=))

$(basename $(basename $(notdir $1)))::	$(addprefix $(basename $(basename $1)).,$(generated_language_suffixes))

clean::
	@rm -f $(addprefix $(dir $1)$(basename $(basename $(notdir $1)))., $(lang_transforms:.TokenTree.xsl=))

clean::
	@rm -f $(addprefix $(basename $(basename $1)).,$(generated_language_suffixes) $(clean_generated_language_suffixes))

endef

$(foreach lang,$(languages),$(eval $(call LANGTEMPLATE,$(lang))))
# $(foreach lang,$(languages),$(info $(call LANGTEMPLATE,$(lang))))


# Run re2c on some text format
$(lang_tmp)/%:	$(lang_tmp)/%.re2c
	re2c $^ --output $@ $(RE2COPTS)

# Drop the final .hex
$(lang_tmp)/%:	$(lang_tmp)/%.hex | $(hex_to_binary)
	./$(hex_to_binary) < "$<" > "$@"



# Slightly messy. The main user of the schema files in this context
# is xmllint, which wants the xml syntax .rng file.
# If the rng is edited directly, that tends to mess up the timestamp
# tracking in make. Therefore going to have schema.xml and schema.rnc as
# the pair of synchronised files, derive schema.rng from whichever is newer
# in a fashion that updates the other, then use the .rng output for linting


# Hacky. "make create_subproject foo bar" will create a directory
# foo_to_bar with some initial boilerplate files in it.
# TODO, some error checking, e.g. that two arguments are passed
ifeq (create_subproject,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "create_subproject"
  CREATE_SUBPROJECT_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(CREATE_SUBPROJECT_ARGS):;@:)
endif
create_subproject: ## Helper for creating makefile subproject boilerplate
	$(eval SRC := $(word 1,$(CREATE_SUBPROJECT_ARGS)))
	$(eval DST := $(word 2,$(CREATE_SUBPROJECT_ARGS)))
	$(eval TAG := $(SRC)_to_$(DST))
	$(eval MK := $(TAG)/$(TAG).mk)
	@echo "Create subproject $(SRC) $(DST)"
	@rm -f $(TAG)/*
	@mkdir -p $(TAG)
	@cp subtransforms/identity.xsl $(TAG)/$(TAG).xsl
	@echo "start = $(SRC)" > $(TAG)/$(SRC).rnc
	@echo "$(SRC) = element $(SRC) {text}" >> $(TAG)/$(SRC).rnc
	@echo "start = $(DST)" > $(TAG)/$(DST).rnc
	@echo "$(DST) = element $(DST) {text}" >> $(TAG)/$(DST).rnc

	@echo '# Expects the input file $$(XMLPipelineWorkDir)/$(TAG)/whatever.$(SRC).xml' > $(MK)
	@echo '# and the output file $$(XMLPipelineWorkDir)/$(TAG)/whatever.$(DST).xml' >> $(MK)

	@echo "" >> $(MK)
	@echo '$$(if $$(XML_Pipeline_Template),,$$(error makefile requires XML_Pipeline_Template))' >> $(MK)

	@echo "" >> $(MK)
	@echo '$(SRC)To$(DST)SrcDir := $(TAG)' >> $(MK)
	@echo '$(SRC)To$(DST)ToXMLWorkDir := $$(XMLPipelineWorkDir)/$$($(SRC)To$(DST)SrcDir)' >> $(MK)

	@echo "" >> $(MK)
	@echo '$$(eval $$(call XML_Pipeline_Template,$$($(SRC)To$(DST)SrcDir),$$($(SRC)To$(DST)ToXMLWorkDir),$(SRC),$(DST)))' >> $(MK)



# Misc xstl helper scripts are accumulating under subtransforms
# Check they're valid xslt, exposed under a top level validate/subtransforms target
SUBTRANSFORMS := $(wildcard subtransforms/*.xsl)
define subtransform_template
.PHONY: validate/$1
validate/$1:	$1  | $(xmllint)
	@$(xmllint) --relaxng xslt.rng "$$<" $$(XMLLINTOPTS) --quiet
endef
$(foreach t,$(SUBTRANSFORMS), $(eval $(call subtransform_template,$(t))))
.PHONY: validate/subtransforms
validate/subtransforms:	$(foreach t,$(SUBTRANSFORMS), validate/$(t))

all::	validate/subtransforms



# arguments
# $1 name of directory to write files into
# $2 from format, requires $(from).rng in source directory
# $3 to format, requires $(to).rng in source directory
# $4 schema for from format
# $5 transform to apply
# $6 schema for to format
# transform will run $2_to_$3.xsl
# Checks the xsl transform and the schemas for validity before running

define XML_Pipeline_Template_Precise
# Multiple definitions of the same target is a hazard here
# Would be better to use a single rule for any schema validation
.PHONY: validateA/$1/$4
validateA/$1/$4:	$4  | $(xmllint)
	@$(xmllint) --relaxng relaxng.rng "$$<" $(XMLLINTOPTS) --quiet

.PHONY: validateB/$1/$5
validateB/$1/$5:	$5  | $(xmllint)
	@$(xmllint) --relaxng xslt.rng "$$<" $(XMLLINTOPTS) --quiet

.PHONY: validateC/$1/$6
validateC/$1/$6:	$6 | $(xmllint)
	@$(xmllint) --relaxng relaxng.rng "$$<" $(XMLLINTOPTS) --quiet

$1/%.$3.prelint.xml:	$1/%.$2.xml $4 $5 $6 validateA/$1/$4 validateB/$1/$5 validateC/$1/$6 validate/subtransforms  | $(xmllint) $(xsltproc)
	@mkdir -p "$$(dir $$@)"
	@$(xmllint) --relaxng $4 "$$<" $(XMLLINTOPTS) --quiet
	@$(xsltproc) $(XSLTPROCOPTS) --output "$$@" $5 "$$<"

# This transform seems to be prone to stack overflows
$1/%.$3.pretty.xml:	$1/%.$3.prelint.xml | $(xsltproc)
	@$(xsltproc) $(XSLTPROCOPTS) --output "$$@" subtransforms/pretty.xsl "$$<" || cp "$$<" "$$@"

$1/%.$3.xml:	$1/%.$3.pretty.xml | $(xmllint)
	@$(xmllint) --relaxng $6 "$$<" $(XMLLINTOPTS) --quiet
	@cp "$$<" "$$@"
endef

# arguments
# $1 name of current pipeline, source directory
# $2 name of directory to write files into
# $3 from format, requires $(from).rng in source directory
# $4 to format, requires $(to).rng in source directory
# transform will run $3_to_$4.xsl
# Checks the xsl transform and the schemas for validity before running

define XML_Pipeline_Template
$(call XML_Pipeline_Template_Precise,$2,$3,$4,$(call get_schema_name, %$1/$3.rng),$1/$3_to_$4.xsl,$(call get_schema_name, %$1/$4.rng))
endef

define XML_Pipeline_Template_Common
$(call XML_Pipeline_Template_Precise,$1,$2,$3,$(call get_schema_name, %common/$2.rng),common/$2_to_$3.xsl,$(call get_schema_name, %common/$3.rng))
endef


XMLPipelineWorkDir := .pipeline

# Creates target: $(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.expressions.xml
# from source:    $(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.raw_sexpr.xml
# Directory contains schema and transforms implementing that
include $(SELF_DIR)raw_sexpr_to_expressions/raw_sexpr_to_expressions.mk

# Create the input file
$(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.raw_sexpr.xml: Lisp/%.scm | $(file_to_cdata) $(xmllint)
	@mkdir -p "$(dir $@)"
	@$(file_to_cdata) < "$<" > "$@"
	@$(xmllint) --relaxng $(call get_schema_name, %raw_sexpr_to_expressions/raw_sexpr.rng) "$@" $(XMLLINTOPTS) --quiet


# Copy the output file out
LispExpressions/%.xml:	$(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.expressions.xml
	@mkdir -p "$(dir $@)"
	@cp "$<" "$@"


# Expressions -> raw sexpr
include $(SELF_DIR)expressions_to_raw_sexpr/expressions_to_raw_sexpr.mk

# Copy the expressions in
$(XMLPipelineWorkDir)/expressions_to_raw_sexpr/%.expressions.xml: LispExpressions/%.xml
	@mkdir -p "$(dir $@)"
	@cp "$<" "$@"

# Extract the sexpr
LispChecked/%.scm:	$(XMLPipelineWorkDir)/expressions_to_raw_sexpr/%.raw_sexpr.xml validate/subtransforms | $(xmllint) $(xsltproc)
	@mkdir -p "$(dir $@)"
	@$(xmllint) --relaxng $(call get_schema_name, %expressions_to_raw_sexpr/raw_sexpr.rng) "$<" $(XMLLINTOPTS) --quiet
	@$(xsltproc) $(XSLTPROCOPTS) --output "$@" subtransforms/drop_outer_element.xsl "$<"


include $(SELF_DIR)ctree_to_csyntax/ctree_to_csyntax.mk
RAW_CTREE := $(filter-out CTree/schemas.xml,$(call rwildcard,CTree/,*.xml))
CSYNTAX := $(subst CTree,CSyntax,$(RAW_CTREE:.xml=.c))

EXTRAS := schemas.xml ctree.rnc csyntax.rnc

$(XMLPipelineWorkDir)/ctree_to_csyntax/%: ctree_to_csyntax/%
	@mkdir -p "$(dir $@)"
	@cp "$<" "$@"

$(XMLPipelineWorkDir)/ctree_to_csyntax/%.ctree.xml: CTree/%.xml $(foreach f,$(EXTRAS),$(XMLPipelineWorkDir)/ctree_to_csyntax/$(f))
	@mkdir -p "$(dir $@)"
	@cp "$<" "$@"

$(CSYNTAX):	CSyntax/%.c:	$(XMLPipelineWorkDir)/ctree_to_csyntax/%.csyntax.xml validate/subtransforms | $(xmllint) $(xsltproc)
	@mkdir -p "$(dir $@)"
	@$(xmllint) --relaxng $(call get_schema_name, %ctree_to_csyntax/csyntax.rng) "$<" $(XMLLINTOPTS) --quiet
	@$(xsltproc) $(XSLTPROCOPTS) --output "$@" subtransforms/drop_outer_element.xsl "$<"

clean::
	rm -rf CSyntax

clean::
	rm -rf LispExpressions $(XMLPipelineWorkDir) LispChecked


all::	$(CSYNTAX)

RAW_SCHEME := $(call rwildcard,Lisp/,*.scm)
all::	$(RAW_SCHEME:Lisp/%.scm=LispChecked/%.scm) $(RAW_SCHEME:Lisp/%.scm=LispExpressions/%.xml) $(CSYNTAX)

include $(SELF_DIR)Planning/Planning.mk

$(eval $(call XML_Pipeline_Template_Precise,.Planning/tmp,cmark,md,$(call get_schema_name, %common/cmark.rng),common/cmark_to_md.xsl,$(call get_schema_name, %common/md.rng)))

$(eval $(call XML_Pipeline_Template_Precise,.Planning/tmp,cmark,html,$(call get_schema_name, %common/cmark.rng),common/cmark_to_html.xsl,$(call get_schema_name, %common/html.rng)))

%.html:	%.html.xml | $(xmllint) $(xsltproc)
	@$(xmllint) --relaxng $(call get_schema_name, %common/html.rng) "$<" $(XMLLINTOPTS) --quiet
	@$(xsltproc) $(XSLTPROCOPTS) --output "$@" xml_to_html.xsl "$<"

LEXER_OBJECTS := $(addprefix $(TOOLS_DIR_OBJ)/,lexer.posix.o lexer.re2.o lexer.re2c.o lexer.interp.o)

MISC_TOOLS_OBJECTS := $(addprefix $(TOOLS_DIR_OBJ)/,stringtable.o intset.o intstack.o intmap.o minilibc/linux.o)

# TODO: automatically detect this
# apt install libre2-dev puts it here under debian
# set to the empty string to disable
RE2LIB := /usr/lib/x86_64-linux-gnu/libre2.a

# WIP lexer/parser constructions. Very much a prototype at present.
arith_tmp := .arith.O
include $(SELF_DIR)arith/arith.mk

arith.lexer:	$(arith_tmp)/arith.lexer.o
	$(CXX) $(CXXFLAGS) $^ $(RE2LIB) -o $@

arith.main:	$(arith_tmp)/arith.lemon.o $(arith_tmp)/arith.main.o
	$(CC) $(CFLAGS) $^ -o $@

arith.stdin_to_tree:	$(arith_tmp)/stdin_to_tree.o $(arith_tmp)/arith.lemon.o $(arith_tmp)/arith.ptree.o $(arith_tmp)/arith.definitions.o $(LEXER_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(RE2LIB) -o $@

arith.tests:	$(arith_tmp)/arith.tests.o $(arith_tmp)/arith.ptree.o $(TOOLS_DIR_OBJ)/generic_ptree.o
	$(CXX) $(CXXFLAGS) $^ $(RE2LIB) -o $@

clean::
	rm -f arith.lemon.c arith.lemon.h arith.lemon.out
	rm -f arith.lexer.xml arith.lexer.cpp arith.lexer.o arith.lexer
	rm -f arith.main arith.stdin_to_tree arith.tests

$(arith_tmp)/arith.prelint.xml:	arith.lexer $(call get_schema_name, %common/TokenList.rng)
	echo "1 + 2 / 1 * (7 % 3)" | ./arith.lexer > $@

$(arith_tmp)/arith.xml:	$(arith_tmp)/arith.prelint.xml
	xmllint --relaxng $(call get_schema_name, %common/TokenList.rng) "$<"
	@cp "$<" "$@"

# Don't want to be using this xsl
arith.txt:	TokenList_to_bytes.xsl $(arith_tmp)/arith.xml
	xsltproc --output $@ $^

clean::
	rm -f arith.txt


# WIP regex operations
regex_tmp := .regex.O
include $(SELF_DIR)regex/regex.mk


# Building auxilary tools out of C.

.PHONY: vendored
vendored:: ## Unpack vendored tools into the source tree

.PHONY: deepclean
deepclean:: ## Remove vendored source code and clean
deepclean:: clean


include $(SELF_DIR)vendored/vendored.mk

# Simple binaries are some single file C files at top level
SIMPLE_TOOLS_BIN := $(lemon) $(makeheaders) $(hex_to_binary) $(file_to_cdata) bin/ptree.tests bin/arena bin/stack

# cmark uses multiple source files, specifically all those under the cmark directory
CMARK_SRC:= $(wildcard $(TOOLS_DIR)/cmark/*.c)

# these have been mangled by the vendored makefile to build as below
LIBXML2_SRC := $(call rwildcard,$(TOOLS_DIR)/libxml2,*.c)
LIBXSLT_SRC := $(call rwildcard,$(TOOLS_DIR)/libxslt,*.c)

LIBWASM3_SRC:= $(call rwildcard,$(TOOLS_DIR)/wasm3/source,*.c)

# All C, C++ get compiled to object files individually
TOOLS_C_SRC := $(call rwildcard,$(TOOLS_DIR),*.c)
TOOLS_CPP_SRC := $(call rwildcard,$(TOOLS_DIR),*.cpp)

# Assumes everything depends on all headers
TOOLS_HDR := $(call rwildcard,$(TOOLS_DIR),*.h) $(call rwildcard,$(TOOLS_DIR),*.hpp) $(call rwildcard,$(TOOLS_DIR),*.inc)
TOOLS_C_OBJ := $(TOOLS_C_SRC:$(TOOLS_DIR)/%.c=$(TOOLS_DIR_OBJ)/%.o)
TOOLS_CPP_OBJ := $(TOOLS_CPP_SRC:$(TOOLS_DIR)/%.cpp=$(TOOLS_DIR_OBJ)/%.o)

$(TOOLS_C_OBJ):	TARGET_CFLAGS := -Wno-covered-switch-default -Wno-unused-function -Wno-unused-const-variable -Wno-null-pointer-arithmetic -Wno-unused-parameter -Wno-unused-variable

$(TOOLS_C_OBJ):	$(TOOLS_DIR_OBJ)/%.o:	$(TOOLS_DIR)/%.c $(TOOLS_HDR)
	@mkdir -p "$(dir $@)"
	@$(CC) $(CFLAGS) $(TARGET_CFLAGS) $< -c -o $@

$(TOOLS_CPP_OBJ):	$(TOOLS_DIR_OBJ)/%.o:	$(TOOLS_DIR)/%.cpp $(TOOLS_HDR)
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CXXFLAGS) $(TARGET_CXXFLAGS) $< -c -o $@

# Build the binaries
$(TOOLS_DIR_BIN):
	@mkdir -p $(TOOLS_DIR_BIN)

CMARK_OBJ := $(CMARK_SRC:$(TOOLS_DIR)/%.c=$(TOOLS_DIR_OBJ)/%.o)
$(cmark):	$(CMARK_OBJ) | $(TOOLS_DIR_BIN)
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Runs unit tests for hashtable
HASHTABLE_SRC := $(addprefix $(TOOLS_DIR)/,hashtable.c intset.c intmap.c stringtable.c intstack.c)
HASHTABLE_OBJ := $(HASHTABLE_SRC:$(TOOLS_DIR)/%.c=$(TOOLS_DIR_OBJ)/%.o)
$(TOOLS_DIR_BIN)/hashtable:	$(HASHTABLE_OBJ) | $(TOOLS_DIR_BIN)
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@


# Doesnt construct a library, directly links against the objects
LIBXML2_OBJ := $(LIBXML2_SRC:$(TOOLS_DIR)/%.c=$(TOOLS_DIR_OBJ)/%.o)
LIBXSLT_OBJ := $(LIBXSLT_SRC:$(TOOLS_DIR)/%.c=$(TOOLS_DIR_OBJ)/%.o)

$(TOOLS_DIR_BIN)/xmllint:	$(TOOLS_DIR_OBJ)/xmllint.o $(LIBXML2_OBJ) | $(TOOLS_DIR_BIN)
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(TOOLS_DIR_BIN)/xsltproc:	$(TOOLS_DIR_OBJ)/xsltproc.o $(LIBXSLT_OBJ) $(LIBXML2_OBJ) | $(TOOLS_DIR_BIN)
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@


LIBWASM3_OBJ := $(LIBWASM3_SRC:$(TOOLS_DIR)/%.c=$(TOOLS_DIR_OBJ)/%.o)
$(TOOLS_DIR_BIN)/wasm3:	$(TOOLS_DIR_OBJ)/wasm3.o $(LIBWASM3_OBJ) | $(TOOLS_DIR_BIN)
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@


$(SIMPLE_TOOLS_BIN):	$(TOOLS_DIR_BIN)/%:	$(TOOLS_DIR_OBJ)/%.o | $(TOOLS_DIR_BIN)
	@$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

tools: ## Create binary tools
tools:	$(SIMPLE_TOOLS_BIN)
tools:	$(TOOLS_DIR_BIN)/cmark $(TOOLS_DIR_BIN)/xmllint $(TOOLS_DIR_BIN)/xsltproc
tools:	$(TOOLS_DIR_BIN)/wasm3
tools:	$(TOOLS_DIR_BIN)/hashtable


clean::
	rm -rf $(TOOLS_DIR_BIN) $(TOOLS_DIR_OBJ)

# Help idea derived from https://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
# Prints the help-text from `target: ## help-text`, slightly reformatted and sorted

HELP_PADDING := 30

.PHONY: awkhelp
awkhelp: ## Write this help using awk
	@echo "awkhelp:"
	@awk 'BEGIN {FS = ":.*#+"}; /^[a-zA-Z_*.-]+:.*## .*$$/ {printf "  %-'$(HELP_PADDING)'s %s\n", $$1, $$2}' \
	$(MAKEFILE_LIST) | \
	sort

.PHONY: sedhelp
sedhelp: ## Write this help using sed
	@echo "sedhelp:"
	@sed -E \
	-e '/^([a-zA-Z_*.-]+::?[ ]*)##[ ]*([^#]*)$$/ !d # grep' \
	-e 's/([a-zA-Z_*.-]+:):?(.*)/  \1\2/ # drop :: and prefix pad' \
	-e ':again s/^([^#]{1,'$(HELP_PADDING)'})##[ ]*([^#]*)$$/\1 ##\2/ # insert a space' \
	-e 't again # do it again (termination is via {1, HELP_PADDING})' \
	-e 's/^([^#]*)##([^#]*)$$/\1\2/ # remove the ##' \
	$(MAKEFILE_LIST) | \
	sort


.PHONY: help
help:  ## Write this help
help:  sedhelp

# At the end to depend on the included makefiles as well as this one
.EXTRA_PREREQS+=$(foreach mk, ${MAKEFILE_LIST},$(abspath ${mk}))
