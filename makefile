# xslt pipeline handling

.SUFFIXES:
MAKEFLAGS += -r
.SECONDARY:
# .DELETE_ON_ERROR:

# SHELL = sh -xv

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

CC := gcc
CXX := g++

C_OR_CXX_FLAGS := -Wall -Wextra -g
CFLAGS := -std=c11 $(C_OR_CXX_FLAGS)
CXXFLAGS := -std=c++14 -Wno-c99-designator $(C_OR_CXX_FLAGS)

# Considering a single source single file approach to tools
file_to_cdata := bin/file_to_cdata
hex_to_binary := bin/hex_to_binary
lemon := bin/lemon
makeheaders := bin/makeheaders

cmark := bin/cmark

# Source under TOOLS_DIR used to make binaries under TOOLS_DIR_BIN
TOOLS_DIR := tools
TOOLS_DIR_BIN := bin


include submakefiles/schemas.mk

# This doens't work as it should - dropdtd does not suppress the warning
# on missing dtd if also doing some other verification
# Patching around by patching cmark/xml.c to not write the DOCTYPE
XMLLINTOPTS := --nonet --huge --noout --dropdtd

# The xml pretty printer tends to trip the depth limit when it's the default 3000
# novalid stops it looking for .dtd files, notably CommonMark.dtd
XSLTPROCOPTS := --huge  --maxdepth 40000 --novalid

# Slightly messy. The main user of the schema files in this context
# is xmllint, which wants the xml syntax .rng file.
# If the rng is edited directly, that tends to mess up the timestamp
# tracking in make. Therefore going to have schema.xml and schema.rnc as
# the pair of synchronised files, derive schema.rng from whichever is newer
# in a fashion that updates the other, then use the .rng output for linting

rwildcard = $(foreach d,$(wildcard $(1)*),$(call rwildcard,$(d)/,$(2)) $(filter $(subst *,%,$(2)),$(d)))

RAW_SCHEME := $(call rwildcard,Lisp/,*.scm)


# Pretty hacky. "make create_subproject foo bar" will create a directory
# foo_to_bar with some initial boilerplate files in it.
# TODO, some error checking, e.g. that two arguments are passed
ifeq (create_subproject,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "create_subproject"
  CREATE_SUBPROJECT_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(CREATE_SUBPROJECT_ARGS):;@:)
endif
create_subproject:
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
validate/$1:	$1
	@xmllint --relaxng xslt.rng "$$<" $$(XMLLINTOPTS) --quiet
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
validateA/$1/$4:	$4
	@xmllint --relaxng relaxng.rng "$$<" $(XMLLINTOPTS) --quiet

.PHONY: validateB/$1/$5
validateB/$1/$5:	$5
	@xmllint --relaxng xslt.rng "$$<" $(XMLLINTOPTS) --quiet

.PHONY: validateC/$1/$6
validateC/$1/$6:	$6
	@xmllint --relaxng relaxng.rng "$$<" $(XMLLINTOPTS) --quiet

$1/%.$3.prelint.xml:	$1/%.$2.xml $4 $5 $6 validateA/$1/$4 validateB/$1/$5 validateC/$1/$6 validate/subtransforms
	@mkdir -p "$$(dir $$@)"
	@xmllint --relaxng $4 "$$<" $(XMLLINTOPTS) --quiet
	@xsltproc $(XSLTPROCOPTS) --output "$$@" $5 "$$<"

$1/%.$3.pretty.xml:	$1/%.$3.prelint.xml
	@xsltproc $(XSLTPROCOPTS) --output "$$@" subtransforms/pretty.xsl "$$<"

$1/%.$3.xml:	$1/%.$3.pretty.xml
	@xmllint --relaxng $6 "$$<" $(XMLLINTOPTS) --quiet
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
$(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.raw_sexpr.xml: Lisp/%.scm $(file_to_cdata)
	@mkdir -p "$(dir $@)"
	@$(file_to_cdata) < "$<" > "$@"
	@xmllint --relaxng $(call get_schema_name, %raw_sexpr_to_expressions/raw_sexpr.rng) "$@" $(XMLLINTOPTS) --quiet


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
LispChecked/%.scm:	$(XMLPipelineWorkDir)/expressions_to_raw_sexpr/%.raw_sexpr.xml validate/subtransforms
	@mkdir -p "$(dir $@)"
	@xmllint --relaxng $(call get_schema_name, %expressions_to_raw_sexpr/raw_sexpr.rng) "$<" $(XMLLINTOPTS) --quiet
	@xsltproc $(XSLTPROCOPTS) --output "$@" subtransforms/drop_outer_element.xsl "$<"


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

$(CSYNTAX):	CSyntax/%.c:	$(XMLPipelineWorkDir)/ctree_to_csyntax/%.csyntax.xml validate/subtransforms
	@mkdir -p "$(dir $@)"
	@xmllint --relaxng $(call get_schema_name, %ctree_to_csyntax/csyntax.rng) "$<" $(XMLLINTOPTS) --quiet
	@xsltproc $(XSLTPROCOPTS) --output "$@" subtransforms/drop_outer_element.xsl "$<"

clean::
	rm -rf CSyntax

clean::
	rm -rf LispExpressions $(XMLPipelineWorkDir) LispChecked

all::	$(CSYNTAX)

all::	$(RAW_SCHEME:Lisp/%.scm=LispChecked/%.scm) $(RAW_SCHEME:Lisp/%.scm=LispExpressions/%.xml) $(CSYNTAX)

include $(SELF_DIR)Planning/Planning.mk


# Global transform rules. Make a file next to the current file with a different suffix.

# Any .md file to commonmark xml
%.cmark.xml:	%.md | $(cmark)
	@./$(cmark) --to xml $^ > $@

%.cmark.html:	%.md | $(cmark)
	@./$(cmark) --to html $^ > $@

$(eval $(call XML_Pipeline_Template_Precise,.Planning/tmp,cmark,md,$(call get_schema_name, %common/cmark.rng),common/cmark_to_md.xsl,$(call get_schema_name, %common/md.rng)))

$(eval $(call XML_Pipeline_Template_Precise,.Planning/tmp,cmark,html,$(call get_schema_name, %common/cmark.rng),common/cmark_to_html.xsl,$(call get_schema_name, %common/html.rng)))

%.html:	%.html.xml
	@xmllint --relaxng $(call get_schema_name, %common/html.rng) "$<" $(XMLLINTOPTS) --quiet
	@xsltproc $(XSLTPROCOPTS) --output "$@" xml_to_html.xsl "$<"

# Building auxilary tools out of C.

# Simple binaries are some single file C files at top level
SIMPLE_TOOLS_BIN := $(lemon) $(makeheaders) $(hex_to_binary) $(file_to_cdata)

# cmark uses multiple source files, specifically all those under the cmark directory
CMARK_SRC:= $(wildcard $(TOOLS_DIR)/cmark/*.c)

# All C, C++ get compiled to object files individually
TOOLS_DIR_OBJ := .$(TOOLS_DIR).O
TOOLS_C_SRC := $(call rwildcard,$(TOOLS_DIR),*.c)
TOOLS_CPP_SRC := $(call rwildcard,$(TOOLS_DIR),*.cpp)

# Assumes everything depends on all headers
TOOLS_HDR := $(call rwildcard,$(TOOLS_DIR),*.h) $(call rwildcard,$(TOOLS_DIR),*.hpp) $(call rwildcard,$(TOOLS_DIR),*.inc)
TOOLS_C_OBJ := $(TOOLS_C_SRC:$(TOOLS_DIR)/%.c=$(TOOLS_DIR_OBJ)/%.o)
TOOLS_CPP_OBJ := $(TOOLS_CPP_SRC:$(TOOLS_DIR)/%.cpp=$(TOOLS_DIR_OBJ)/%.o)

$(TOOLS_C_OBJ):	$(TOOLS_DIR_OBJ)/%.o:	$(TOOLS_DIR)/%.c $(TOOLS_HDR)
	@mkdir -p "$(dir $@)"
	@$(CC) $(CFLAGS) $< -c -o $@

$(TOOLS_CPP_OBJ):	$(TOOLS_DIR_OBJ)/%.o:	$(TOOLS_DIR)/%.cpp $(TOOLS_HDR)
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CXXFLAGS) $< -c -o $@

CMARK_OBJ := $(CMARK_SRC:$(TOOLS_DIR)/%.c=$(TOOLS_DIR_OBJ)/%.o)
$(cmark):	$(CMARK_OBJ)
	@mkdir -p "$(dir $@)"
	@$(CC) $(CFLAGS) $^ -o $@

$(SIMPLE_TOOLS_BIN):	$(TOOLS_DIR_BIN)/%:	$(TOOLS_DIR_OBJ)/%.o
	@mkdir -p "$(dir $@)"
	@$(CC) $(CFLAGS) $< -o $@

tools:	$(SIMPLE_TOOLS_BIN) $(TOOLS_DIR_BIN)/cmark

clean::
	rm -rf $(TOOLS_DIR_BIN) $(TOOLS_DIR_OBJ)





# At the end to depend on the included makefiles as well as this one
.EXTRA_PREREQS+=$(foreach mk, ${MAKEFILE_LIST},$(abspath ${mk}))
