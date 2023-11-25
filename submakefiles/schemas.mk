# The idea here is to allow relaxng schemas in compact or xml formats
# That's reasonably messy in make so is factored out. This file defines
# various targets and variables prefixed SCHEMA and one function that takes
# a %-prefixed name of a schema from somewhere in the MAKEFILE_DIR rooted tree:
# $(call get_schema_name, %derived.rnc)
# that returns the name of an up to date copy of that target, even if the only
# source for that schema in tree is a .rng format

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
SCHEMA_UPDATED_RNC := $(call SCHEMA_CHDIR, $(SCHEMA_RAW_RNC_SOURCE:.rnc=.rng))
SCHEMA_UPDATED_RNG := $(call SCHEMA_CHDIR, $(SCHEMA_RAW_RNG_SOURCE:.rng=.rng))

# Going to make one file for each schema under XML_DIR
SCHEMA_UPDATED := $(sort $(SCHEMA_UPDATED_RNC) $(SCHEMA_UPDATED_RNG))

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
#	@echo "Updating $@ from $<"
	@if ! test -s $< ; then trang -Irnc -Orng $(<:.rng=.rnc) $< ; fi
	@mkdir -p $(dir $@)
	@cp $< $@
	@$(if $(filter $@, $(SCHEMA_UPDATED_RNG)), @trang -Irng -Ornc $@ $(<:.rng=.rnc))
	@touch $@

$(SCHEMA_UPDATED_RNC):: $(SCHEMA_XML_DIR)%.rng:	%.rnc
#	@echo "Updating $@ from $<"
	@if ! test -s $< ; then trang -Irng -Ornc $(<:.rnc=.rng) $< ; fi
	@mkdir -p $(dir $@)
	@trang -Irnc -Orng $< $@
	@$(if $(filter $@, $(SCHEMA_UPDATED_RNG)), cp $@ $(<:.rnc=.rng))
	@touch $@

# Made the XML dir one, extract the two formats from it
# Could simplify slightly by deleting the XML subdirectory

$(SCHEMA_DERIVED_RNG): $(SCHEMA_RNG_DIR)%.rng:	$(SCHEMA_XML_DIR)%.rng
	@mkdir -p $(dir $@)
	@cp $< $@

$(SCHEMA_DERIVED_RNC): $(SCHEMA_RNC_DIR)%.rnc:	$(SCHEMA_XML_DIR)%.rng
	@mkdir -p $(dir $@)
	@trang -Irng -Ornc $< $@


# Final result of all this, an accessor for a name of an up to date schema file
# in either format
get_schema_name = $(filter $1, $(SCHEMA_DERIVED_RNG)$(SCHEMA_DERIVED_RNC))

