
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
PRIMARY_SUFFIX=rng
SECONDARY_SUFFIX=rnc
DERIVED_SUFFIX=xml

secondary_from_primary = trang -Irng -Ornc $2 $1
primary_from_secondary = trang -Irnc -Orng $2 $1

RAW_PRIMARY := $(wildcard *.$(PRIMARY_SUFFIX))
RAW_SECONDARY := $(wildcard *.$(SECONDARY_SUFFIX))

DERIVED_SOURCE := $(sort \
	$(RAW_PRIMARY:$(PRIMARY_SUFFIX)=$(DERIVED_SUFFIX)) \
	$(RAW_SECONDARY:$(SECONDARY_SUFFIX)=$(DERIVED_SUFFIX)))

DERIVED_SOURCE_GIVEN_PRIMARY = $(RAW_PRIMARY:$(PRIMARY_SUFFIX)=$(DERIVED_SUFFIX))
DERIVED_SOURCE_GIVEN_SECONDARY = $(RAW_SECONDARY:$(SECONDARY_SUFFIX)=$(DERIVED_SUFFIX))

# Source that only has one of the two potential forms available
# The dependency tracking is probably wrong in the transient state
DERIVED_PRIMARY = $(filter-out $(RAW_PRIMARY),$(RAW_SECONDARY:$(SECONDARY_SUFFIX)=$(PRIMARY_SUFFIX)))
DERIVED_SECONDARY = $(filter-out $(RAW_SECONDARY),$(RAW_PRIMARY:$(PRIMARY_SUFFIX)=$(SECONDARY_SUFFIX)))


_pos = $(if $(findstring $1,$2), $(call _pos,$1, $(wordlist 2,$(words $2),$2),x $3) ,$3)
pos = $(words $(call _pos,$1,$2))

cdr = $(wordlist 2, $(words $1), $1)
rdc = $(wordlist 1, $(words $(call cdr, $1)), $1)

below_index = $(call rdc, $(wordlist 1,$1,$2))
at_index = $(word $1, $2)
above_index = $(call cdr, $(wordlist $1,$(words $2), $2))


lookup = $(word $(call pos,$1,$2),$3)
remove = $(call below_index, $1, $2) $(call above_index, $1, $2)
insert = $(call below_index, $1, $3) $2 $(call above_index, $1, $3)

# trang can't cope with xslt
XSD_FILES := $(filter-out xslt.xsd, $(PREFERRED_SOURCE:.pref=.xsd))


RAW_SCHEME := $(wildcard Lisp/*.scm)
MOD_XML := $(RAW_SCHEME:Lisp/%.scm=LispXML/%.xml)


LispXML:
	@mkdir -p $@


XMLTmpDir := .tmpxml
MkdirXMLTmpDir: | LispXML
	@mkdir -p LispXML/$(XMLTmpDir)


define XML_Transform_Template
# arguments fromdir 1, from 2, todir 3, to 4
LispXML/$3/%.$4.xml:	LispXML/$1/%.$2.xml | $2.rng $2_to_$4.xsl $4.rng
	@mkdir -p LispXML/$3
	@xmllint --relaxng $2.rng $$< --noout --quiet
	@xsltproc --output $$@ $2_to_$4.xsl $$^
#	@xmllint --relaxng $4.rng $$@ --noout --quiet
	@xsltproc --output $$@ --maxdepth 40000 pretty.xsl $$@
	@xmllint --relaxng $4.rng $$@ --noout --quiet
endef



$(eval $(call XML_Transform_Template,$(XMLTmpDir),raw,$(XMLTmpDir),list))
$(eval $(call XML_Transform_Template,$(XMLTmpDir),list,$(XMLTmpDir),symbols))
$(eval $(call XML_Transform_Template,$(XMLTmpDir),symbols,$(XMLTmpDir),derived))
$(eval $(call XML_Transform_Template,$(XMLTmpDir),symbols,$(XMLTmpDir),expressions))
$(eval $(call XML_Transform_Template,$(XMLTmpDir),expressions,.regen,symbols))
$(eval $(call XML_Transform_Template,.regen,symbols,.regen,derived))


# arguments
# $1 name of current pipeine, source directory
# $2 name of directory to write files into to
# $3 from format, requires $(from).rng in source directory
# $4 to format, requires $(to).rng in source directory
# transform will run $3_to_$4.xsl
# The rnc/rng conversions aren't properly wired up yet
define XML_Pipeline_Template

$2/%.$4.xml:	$2/%.$3.xml | $(call get_schema_name, %$1/$3.rng) $1/$3_to_$4.xsl $(call get_schema_name, %$1/$4.rng)
	@mkdir -p $2
	@xmllint --relaxng $(call get_schema_name, %$1/$3.rng) $$< --noout --quiet
	xsltproc --output $$@ $1/$3_to_$4.xsl $$^
	@xmllint --relaxng $(call get_schema_name, %$1/$4.rng) $$@ --noout --quiet
	@xsltproc --output $$@ --maxdepth 40000 pretty.xsl $$@
	@xmllint --relaxng $(call get_schema_name, %$1/$4.rng) $$@ --noout --quiet
endef


XMLPipelineWorkDir := pipeline


# Creates target: $(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.expressions.xml
# from source:    $(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.raw_sexpr.xml
# Directory contains schema and transforms implementing that
include $(SELF_DIR)raw_sexpr_to_expressions/raw_sexpr_to_expressions.mk

# Create the input file
$(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.raw_sexpr.xml: Lisp/%.scm
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $@
	@echo '<RawText xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">' >> $@
	@echo '<![CDATA[' >> $@
	@cat $< >> $@
	@echo ']]></RawText>' >> $@
	@xmllint --relaxng raw.rng $@ --noout --quiet

# Copy the output file out
LispWIP/%.xml:	$(XMLPipelineWorkDir)/raw_sexpr_to_expressions/%.expressions.xml
	@mkdir -p LispWIP
	cp $< $@


LispXML/$(XMLTmpDir)/%.raw.xml:	Lisp/%.scm | MkdirXMLTmpDir raw.rng
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $@
	@echo '<RawText xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">' >> $@
	@echo '<![CDATA[' >> $@
	@cat $< >> $@
	@echo ']]></RawText>' >> $@
	@xmllint --relaxng raw.rng $@ --noout --quiet

$(MOD_XML):	LispXML/%.xml:	LispXML/$(XMLTmpDir)/%.derived.xml | LispXML
	@cp $< $@


all::	$(RAW_SCHEME:Lisp/%.scm=LispWIP/%.xml) .schema/rng/raw_sexpr_to_expressions/list.rng

# Only got one of the files, can build the other from it
$(DERIVED_SECONDARY):	%.$(SECONDARY_SUFFIX):	%.$(PRIMARY_SUFFIX)
	$(call secondary_from_primary,$@,$^)

$(DERIVED_PRIMARY):	%.$(PRIMARY_SUFFIX):	%.$(SECONDARY_SUFFIX)
	$(call primary_from_secondary,$@,$^)

# These only exist for source files that existed before the makefile invocation
PRIMARY_NEWER := $(DERIVED_SOURCE)
$(DERIVED_SOURCE_GIVEN_PRIMARY):: %.$(DERIVED_SUFFIX):	%.$(PRIMARY_SUFFIX)
	$(eval index=$(call pos,$@,$(DERIVED_SOURCE)))
	$(eval PRIMARY_NEWER=$(call insert,$(index),$^,$(PRIMARY_NEWER)))

SECONDARY_NEWER := $(DERIVED_SOURCE)
$(DERIVED_SOURCE_GIVEN_SECONDARY):: %.$(DERIVED_SUFFIX):	%.$(SECONDARY_SUFFIX)
	$(eval index=$(call pos,$@,$(DERIVED_SOURCE)))
	$(eval SECONDARY_NEWER=$(call insert,$(index),$^,$(SECONDARY_NEWER)))

# See whether either dependency warrants rebuilding this target
$(DERIVED_SOURCE):: %.$(DERIVED_SUFFIX) :
	$(eval index=$(call pos,$@,$(DERIVED_SOURCE)))

	$(eval primary=$*.$(PRIMARY_SUFFIX))
	$(eval secondary = $*.$(SECONDARY_SUFFIX))

#	Test whether either candidate dependency is newer than this target
	$(eval from_primary=$(findstring $(primary), $(word $(index),$(PRIMARY_NEWER))))
	$(eval from_secondary=$(findstring $(secondary), $(word $(index),$(SECONDARY_NEWER))))

#	Not totally sure this is worth the bother relative to writing to $@ directly
#	@$(eval tmp=$(shell mktemp -t $@.XXXXXX))

	$(if $(from_primary), \
		cp $(primary) $@, \
		$(if $(from_secondary), $(call primary_from_secondary,$@,$(secondary))))

#	If this was updated by either file, update the other from it
	$(if $(from_primary), \
		$(call secondary_from_primary,$(secondary),$@), \
		$(if $(from_secondary), cp $@ $(primary)))

#	Fix up the timestamp so current target still looks newer than those it just rebuilt
	$(if $(or $(from_primary), $(from_secondary)), touch $@, @rm $@)

xsd_from_rng = trang -Irng -Oxsd $2 $1
$(XSD_FILES):	%.xsd:	%.rng
	$(call xsd_from_rng,$@,$^)


validate:	$(RAW_PRIMARY) $(DERIVED_PRIMARY) | relaxng.rng
	@xmllint --relaxng relaxng.rng $^ --noout


clean::
	@rm -f *.$(DERIVED_SUFFIX)
	rm -rf LispXML LispXML/$(XMLTmpDir)

# good if files are all well formed
hazardous_rebuild:
	$(MAKE)
	rm -rf *.$(DERIVED_SUFFIX) *.$(SECONDARY_SUFFIX)
	$(MAKE)
	rm -rf *.$(DERIVED_SUFFIX) *.$(PRIMARY_SUFFIX)
	$(MAKE)
	rm -rf *.$(DERIVED_SUFFIX)


# At the end to depend on the included makefiles as well as this one
.EXTRA_PREREQS+=$(foreach mk, ${MAKEFILE_LIST},$(abspath ${mk}))
