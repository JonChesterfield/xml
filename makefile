# xslt pipeline handling

.SUFFIXES:
MAKEFLAGS += -r
.SECONDARY:
# .DELETE_ON_ERROR:

# SHELL = sh -xv

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

all::

include submakefiles/schemas.mk

XMLLINTOPTS := --huge --noout
XSLTPROCOPTS := --huge # --maxdepth 40000

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
# $1 name of current pipeine, source directory
# $2 name of directory to write files into to
# $3 from format, requires $(from).rng in source directory
# $4 to format, requires $(to).rng in source directory
# transform will run $3_to_$4.xsl
# Checks the xsl transform and the schemas for validity before running

# Same four arguments as XML_Pipeline_Template then three abbreviations
# for the two schemas and the transform
define XML_Pipeline_Template_Lemma

.PHONY: validateA/$5
validateA/$5:	$5
	@xmllint --relaxng relaxng.rng "$$<" $(XMLLINTOPTS) --quiet

.PHONY: validateB/$6
validateB/$6:	$6
	@xmllint --relaxng xslt.rng "$$<" $(XMLLINTOPTS) --quiet

.PHONY: validateC/$7
validateC/$7:	$7
	@xmllint --relaxng relaxng.rng "$$<" $(XMLLINTOPTS) --quiet

$2/%.$4.prelint.xml:	$2/%.$3.xml $5 $6 $7 | validateA/$5 validateB/$6 validateC/$7
	@mkdir -p "$$(dir $$@)"
	@xmllint --relaxng $5 "$$<" $(XMLLINTOPTS) --quiet
	@xsltproc $(XSLTPROCOPTS) --output "$$@" $6 "$$<"

$2/%.$4.xml:	$2/%.$4.prelint.xml
	@xmllint --relaxng $7 "$$<" $(XMLLINTOPTS) --quiet
	@cp "$$<" "$$@"
endef

define XML_Pipeline_Template
$(call XML_Pipeline_Template_Lemma,$1,$2,$3,$4,$(call get_schema_name, %$1/$3.rng),$1/$3_to_$4.xsl,$(call get_schema_name, %$1/$4.rng))
endef

XMLPipelineWorkDir := .pipeline

# Creates target: $(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.expressions.xml
# from source:    $(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.raw_sexpr.xml
# Directory contains schema and transforms implementing that
include $(SELF_DIR)raw_sexpr_to_expressions/raw_sexpr_to_expressions.mk

# Create the input file
$(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.raw_sexpr.xml: Lisp/%.scm
	@mkdir -p "$(dir $@)"
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > "$@"
	@echo '<RawText xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">' >> "$@"
	@echo '<![CDATA[' >> "$@"
	@cat "$<" >> "$@"
	@echo ']]></RawText>' >> "$@"
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


# At the end to depend on the included makefiles as well as this one
.EXTRA_PREREQS+=$(foreach mk, ${MAKEFILE_LIST},$(abspath ${mk}))
