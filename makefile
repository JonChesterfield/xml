
.SUFFIXES:
.EXTRA_PREREQS:= $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFLAGS += -r

all::

PRIMARY_SUFFIX=rng
SECONDARY_SUFFIX=rnc
DERIVED_SUFFIX=pref

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


all::	$(PREFERRED_SOURCE) $(DERIVED_RNC) $(DERIVED_RNG)

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
	@$(eval tmp=$(shell mktemp -t $@.XXXXXX))

	$(if $(from_primary), \
		cp $(primary) $(tmp), \
		$(if $(from_secondary), $(call primary_from_secondary,$(tmp),$(secondary))))

#	If this was updated by either file, update the other from it
	$(if $(from_primary), \
		$(call secondary_from_primary,$(secondary),$(tmp)), \
		$(if $(from_secondary), cp $(tmp) $(primary)))

xsd_from_rng = trang -Irng -Oxsd $2 $1
$(XSD_FILES):	%.xsd:	%.rng
	$(call xsd_from_rng,$@,$^)

#	Fix up the timestamp so current target still looks newer than those it just rebuilt
	$(if $(or $(from_primary), $(from_secondary)), touch $(tmp) && mv $(tmp) $@, @rm $(tmp))


validate:	$(RAW_PRIMARY) $(DERIVED_PRIMARY) | relaxng.rng
	@xmllint --relaxng relaxng.rng $^ --noout


clean::
	rm -f *.$(DERIVED_SUFFIX)

# good if files are all well formed
hazardous_rebuild:
	$(MAKE)
	rm -rf *.$(DERIVED_SUFFIX) *.$(SECONDARY_SUFFIX)
	$(MAKE)
	rm -rf *.$(DERIVED_SUFFIX) *.$(PRIMARY_SUFFIX)
	$(MAKE)
	rm -rf *.$(DERIVED_SUFFIX)

