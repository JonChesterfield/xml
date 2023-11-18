
.SUFFIXES:
.EXTRA_PREREQS:= $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFLAGS += -r

all::

PRIMARY_SUFFIX=rng
SECONDARY_SUFFIX=rnc
DERIVED_SUFFIX=pref

RAW_RNG := $(wildcard *.$(PRIMARY_SUFFIX))
RAW_RNC := $(wildcard *.$(SECONDARY_SUFFIX))

ALL_RNG_OR_RNC := $(sort $(RAW_RNG:.$(PRIMARY_SUFFIX)=) $(RAW_RNC:.$(SECONDARY_SUFFIX)=))

PREFERRED_SOURCE := $(ALL_RNG_OR_RNC:=.$(DERIVED_SUFFIX))

PREFERRED_SOURCE_GIVEN_PRIMARY = $(RAW_RNG:.$(PRIMARY_SUFFIX)=.$(DERIVED_SUFFIX))
PREFERRED_SOURCE_GIVEN_SECONDARY = $(RAW_RNC:.$(SECONDARY_SUFFIX)=.$(DERIVED_SUFFIX))

PREFERRED_MISSING_FROM_GIVEN_RNG = $(filter-out $(PREFERRED_SOURCE_GIVEN_PRIMARY),$(PREFERRED_SOURCE))
PREFERRED_MISSING_FROM_GIVEN_RNC = $(filter-out $(PREFERRED_SOURCE_GIVEN_SECONDARY),$(PREFERRED_SOURCE))

DERIVED_RNG = $(PREFERRED_MISSING_FROM_GIVEN_RNG:.$(DERIVED_SUFFIX)=.$(PRIMARY_SUFFIX))
DERIVED_RNC = $(PREFERRED_MISSING_FROM_GIVEN_RNC:.$(DERIVED_SUFFIX)=.$(SECONDARY_SUFFIX))

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

secondary_from_primary = trang -Irng -Ornc $2 $1
primary_from_secondary = trang -Irnc -Orng $2 $1




all::	$(PREFERRED_SOURCE) $(DERIVED_RNC) $(DERIVED_RNG) # validate

# Only got one of the files, build the other from it
$(DERIVED_RNC):	%.$(SECONDARY_SUFFIX):	%.$(PRIMARY_SUFFIX)
	$(call secondary_from_primary,$@,$^)

$(DERIVED_RNG):	%.$(PRIMARY_SUFFIX):	%.$(SECONDARY_SUFFIX)
	$(call primary_from_secondary,$@,$^)

PRIMARY_NEWER := $(PREFERRED_SOURCE)
SECONDARY_NEWER := $(PREFERRED_SOURCE)

# Make executes these in order of source and they overwrite the same
# variable, so second one wins if both apply
$(PREFERRED_SOURCE_GIVEN_SECONDARY):: %.$(DERIVED_SUFFIX):	%.$(SECONDARY_SUFFIX)
	$(eval index=$(call pos,$@,$(PREFERRED_SOURCE)))
	$(eval SECONDARY_NEWER=$(call insert,$(index),$^,$(SECONDARY_NEWER)))

$(PREFERRED_SOURCE_GIVEN_PRIMARY):: %.$(DERIVED_SUFFIX):	%.$(PRIMARY_SUFFIX)
	$(eval index=$(call pos,$@,$(PREFERRED_SOURCE)))
	$(eval PRIMARY_NEWER=$(call insert,$(index),$^,$(PRIMARY_NEWER)))

$(PREFERRED_SOURCE):: %.$(DERIVED_SUFFIX) :
	$(eval index=$(call pos,$@,$(PREFERRED_SOURCE)))

	$(eval primary=$*.$(PRIMARY_SUFFIX))
	$(eval secondary = $*.$(SECONDARY_SUFFIX))

	$(eval from_primary=$(findstring $(primary), $(word $(index),$(PRIMARY_NEWER))))
	$(eval from_secondary=$(findstring $(secondary), $(word $(index),$(SECONDARY_NEWER))))

	$(if $(from_primary), cp $(primary) $@)
	$(if $(from_secondary), $(call primary_from_secondary,$@,$(secondary)))

# If it was built from the secondary file, update the primary from this
	$(if $(and $(from_secondary), \
		$(findstring $@, $(PREFERRED_SOURCE_GIVEN_PRIMARY))),\
		cp $@ $(primary) && touch $@)

# If it was built from the primary file, update the secondary
	$(if $(and $(from_primary), \
		$(findstring $@, $(PREFERRED_SOURCE_GIVEN_SECONDARY))),\
		$(call secondary_from_primary,$(secondary),$@) && touch $@)

validate:	$(RAW_RNG) $(DERIVED_RNG) | relaxng.rng
	@xmllint --relaxng relaxng.rng $^ --noout



clean::
	rm -f *.$(DERIVED_SUFFIX)


