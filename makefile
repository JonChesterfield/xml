
.SUFFIXES:
MAKEFLAGS += -r
.SECONDARY:
# .DELETE_ON_ERROR:

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

all::

include submakefiles/schemas.mk


# Slightly messy. The main user of the schema files in this context
# is xmllint, which wants the xml syntax .rng file.
# If the rng is edited directly, that tends to mess up the timestamp
# tracking in make. Therefore going to have schema.xml and schema.rnc as
# the pair of synchronised files, derive schema.rng from whichever is newer
# in a fashion that updates the other, then use the .rng output for linting


rwildcard = $(foreach d,$(wildcard $(1)*),$(call rwildcard,$(d)/,$(2)) $(filter $(subst *,%,$(2)),$(d)))

RAW_SCHEME := $(call rwildcard,Lisp/,*.scm)


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
	@xmllint --relaxng relaxng.rng $$< --noout --quiet

.PHONY: validateB/$6
validateB/$6:	$6
	@xmllint --relaxng xslt.rng $$< --noout --quiet

.PHONY: validateC/$7
validateC/$7:	$7
	@xmllint --relaxng relaxng.rng $$< --noout --quiet

# Formatting the xml messes up raw_sexpr handling
$2/%.$4.xml:	$2/%.$3.xml $5 $6 $7 | validateA/$5 validateB/$6 validateC/$7
	@mkdir -p $$(dir $$@)
	@xmllint --relaxng $5 $$< --noout --quiet
	@xsltproc --output $$@ $6 $$<
	@xmllint --relaxng $7 $$@ --noout --quiet
#	@xsltproc --output $$@ --maxdepth 40000 pretty.xsl $$@
#	@xmllint --relaxng $7 $$@ --noout --quiet

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
	@mkdir -p $(dir $@)
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $@
	@echo '<RawText xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">' >> $@
	@echo '<![CDATA[' >> $@
	@cat $< >> $@
	@echo ']]></RawText>' >> $@
	@xmllint --relaxng raw.rng $@ --noout --quiet

# Copy the output file out
LispExpressions/%.xml:	$(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.expressions.xml
	@mkdir -p $(dir $@)
	@cp $< $@

include $(SELF_DIR)expressions_to_raw_sexpr/expressions_to_raw_sexpr.mk

# Copy the expressions in
$(XMLPipelineWorkDir)/expressions_to_raw_sexpr/%.expressions.xml: LispExpressions/%.xml
	@mkdir -p $(dir $@)
	@cp $< $@

# Extract the sexpr
.PHONY: validate/raw_sexpr_to_sexpr.xsl
validate/raw_sexpr_to_sexpr.xsl:	raw_sexpr_to_sexpr.xsl
	@xmllint --relaxng xslt.rng $< --noout --quiet

LispChecked/%.scm:	$(XMLPipelineWorkDir)/expressions_to_raw_sexpr/%.raw_sexpr.xml raw_sexpr_to_sexpr.xsl | validate/raw_sexpr_to_sexpr.xsl
	@mkdir -p $(dir $@)
	@xmllint --relaxng raw.rng $< --noout --quiet
	@xsltproc --output $@ raw_sexpr_to_sexpr.xsl $<


include $(SELF_DIR)ctree_to_csyntax/ctree_to_csyntax.mk
RAW_CTREE := $(call rwildcard,CTree/,*.xml)
CSYNTAX := $(subst CTree,CSyntax,$(RAW_CTREE:.xml=.c))

$(info $(RAW_CTREE))
$(info $(CSYNTAX))

$(XMLPipelineWorkDir)/ctree_to_csyntax/%.ctree.xml: CTree/%.xml
	@mkdir -p $(dir $@)
	@cp $< $@

$(CSYNTAX):	CSyntax/%.c:	$(XMLPipelineWorkDir)/ctree_to_csyntax/%.csyntax.xml
	@mkdir -p $(dir $@)
	@cp $< $@

clean:: CSyntax

clean::
	rm -rf LispExpressions $(XMLPipelineWorkDir) LispChecked

all::	$(CSYNTAX)

# all::	$(RAW_SCHEME:Lisp/%.scm=LispChecked/%.scm) $(RAW_SCHEME:Lisp/%.scm=LispExpressions/%.xml) $(CSYNTAX)


# At the end to depend on the included makefiles as well as this one
.EXTRA_PREREQS+=$(foreach mk, ${MAKEFILE_LIST},$(abspath ${mk}))
