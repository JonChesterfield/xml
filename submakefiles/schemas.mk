# The idea here is to allow relax ng schemas in compact or xml formats.
# If both exist, editing either will update the other.
# If only one format exists the rest of the system can use either format.
# On ambiguity on what to do, the xml format schema wins.
# Driven by suffix, *.rng is xml syntax and *.rnc is compact syntax.
# 
# This file defines one function that takes a %-prefixed name of a schema from
# somewhere in the MAKEFILE_DIR rooted tree:
# $(call get_schema_name, %derived.rnc)
# The call returns the name of an up to date copy of that target, even if the only
# source for that schema in tree is in the the other format, which can be used
# for xmllint etc
#
# As a special case, if both exist and one is empty, it'll be updated from the other.
# This means one can "fix" a failed partial edit by clearing the current file and
# calling make to clobber it with the previous version. It also means one can create
# a file in the other format, next to the current one, populate that with make, edit
# it with changes reflected in the original, then delete it when done.
#
# If both formats are present, running this script is prone to putting them both in
# the output format of trang, which is roughly a pretty-printed / canonical form.

# Have the caller specify the root of the tree so this file can be hidden
# under a subdirectory. Will only find schema under this root.
$(if $(MAKEFILE_DIR),,$(error require definition for makefile_dir))

# Go in search of files that look like schema in .rnc or .rng format
# Using $(SCHEMA_HIDDEN_DIR) as a working directory, the exported function
# returns names of targets under said working directory and clean deletes it
SCHEMA_HIDDEN_DIR := .schema/
SCHEMA_XML_DIR := $(SCHEMA_HIDDEN_DIR)xml/
SCHEMA_RNG_DIR := $(SCHEMA_HIDDEN_DIR)rng/
SCHEMA_RNC_DIR := $(SCHEMA_HIDDEN_DIR)rnc/

clean::
	rm -rf $(SCHEMA_HIDDEN_DIR)

SCHEMA_RWILDCARD = $(foreach d,$(wildcard $(1)*),$(call SCHEMA_RWILDCARD,$(d)/,$(2)) $(filter $(subst *,%,$(2)),$(d)))

# Ignore any under the codegen dir so running repeatedly doesn't accumulate more
SCHEMA_RAW_RNC_SOURCE := $(filter-out $(MAKEFILE_DIR)/$(SCHEMA_HIDDEN_DIR)%, $(call SCHEMA_RWILDCARD,$(MAKEFILE_DIR)/,*.rnc))
SCHEMA_RAW_RNG_SOURCE := $(filter-out $(MAKEFILE_DIR)/$(SCHEMA_HIDDEN_DIR)%, $(call SCHEMA_RWILDCARD,$(MAKEFILE_DIR)/,*.rng))

# Derive names under XML_DIR
SCHEMA_CHDIR =	$(subst $(MAKEFILE_DIR)/, $(SCHEMA_XML_DIR), $1)
SCHEMA_UPDATED_RNG := $(call SCHEMA_CHDIR, $(SCHEMA_RAW_RNG_SOURCE))
SCHEMA_UPDATED_RNC := $(call SCHEMA_CHDIR, $(SCHEMA_RAW_RNC_SOURCE:.rnc=.rng))

# Going to make one file for each schema under XML_DIR
SCHEMA_UPDATED := $(sort $(SCHEMA_UPDATED_RNG) $(SCHEMA_UPDATED_RNC))

# And then going to extract a RNG and a RNC format from it
SCHEMA_DERIVED_RNG := $(subst $(SCHEMA_XML_DIR), $(SCHEMA_RNG_DIR), $(SCHEMA_UPDATED))
SCHEMA_DERIVED_RNC := $(patsubst %.rng, %.rnc, $(subst $(SCHEMA_XML_DIR), $(SCHEMA_RNC_DIR), $(SCHEMA_UPDATED)))

# If both formats exist for a given schema, update the other one
# This is prone to putting them in the format trang pretty prints, which seems fine
# Note that if the rng and the rnc are both newer than the target tree, the rng format wins
# as it runs first and overwrites the other one
# The double colon :: is why this works out, along with not telling make that
# running the receipe clobbers the other candidate input file

$(SCHEMA_UPDATED_RNG):: $(SCHEMA_XML_DIR)%.rng:	%.rng
	@mkdir -p $(dir $@)
#	@echo "Updating $@ from $<"
	@$(if $(filter $@, $(SCHEMA_UPDATED_RNG)), @if ! test -s $< ; then trang -Irnc -Orng $(<:.rng=.rnc) $< ; fi)
	@cp $< $@
	@$(if $(filter $@, $(SCHEMA_UPDATED_RNG)), @trang -Irng -Ornc $@ $(<:.rng=.rnc))
	@touch $@

$(SCHEMA_UPDATED_RNC):: $(SCHEMA_XML_DIR)%.rng:	%.rnc
	@mkdir -p $(dir $@)
#	@echo "Updating $@ from $<"
	@$(if $(filter $@, $(SCHEMA_UPDATED_RNG)), @if ! test -s $< ; then trang -Irng -Ornc $(<:.rnc=.rng) $< ; fi)
	@trang -Irnc -Orng $< $@
	@$(if $(filter $@, $(SCHEMA_UPDATED_RNG)), cp $@ $(<:.rnc=.rng))
	@touch $@

# Made the single up to date file from one or both of the inputs. Derive the
# output in each format from it.

$(SCHEMA_DERIVED_RNG): $(SCHEMA_RNG_DIR)%.rng:	$(SCHEMA_XML_DIR)%.rng
	@mkdir -p $(dir $@)
	@cp $< $@

$(SCHEMA_DERIVED_RNC): $(SCHEMA_RNC_DIR)%.rnc:	$(SCHEMA_XML_DIR)%.rng
	@mkdir -p $(dir $@)
	@trang -Irng -Ornc $< $@

# Final result of all this, an accessor for a name of an up to date schema file
# in either format
get_schema_name = $(filter $1, $(SCHEMA_DERIVED_RNG)$(SCHEMA_DERIVED_RNC))

