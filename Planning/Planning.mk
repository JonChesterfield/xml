
planning_src := Planning
WORKDIR := .$(planning_src)/tmp


# Most of the complexity here is about spaces in filenames. Could escape more aggressively.
# Careful handling of spaces in filenames. Makefile uses whitespace delimited lists in most places.
# First, find all the files and replace space with a character makefile considers less special

SPACE_ESC := +
SPACE_ESCAPED_SRC := $(shell find "$(planning_src)" -type f -name "*md" | sed 's/ /$(SPACE_ESC)/g')

# Second, turn that back into a list of real filenames, where the spaces are escaped
# Can't do very much with that list but it can be used as a dependency on other targets
RENAMEDSRC := $(patsubst $(planning_src)/%,$(WORKDIR)/%,$(SPACE_ESCAPED_SRC))

$(WORKDIR):
	@mkdir -p $(WORKDIR)


# Seems to be difficult to rely on a single file containing spaces and easy to depend on all of them
ALL_SRC_WITH_SPACES := $(subst $(SPACE_ESC),\ ,$(SPACE_ESCAPED_SRC))
$(RENAMEDSRC):	$(WORKDIR)/%.md:	$(ALL_SRC_WITH_SPACES) | $(WORKDIR)
#	Thus retrieve the specific element in ALL_SRC_WITH_SPACES that is of interest
	$(eval bs_spaces := $(planning_src)/$(subst $(SPACE_ESC),\ ,$*.md))
	cmp $(bs_spaces) $@ >/dev/null 2>&1 || cp $(bs_spaces) $@


clean::
	rm -rf $(WORKDIR)

planning:	$(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.cmark.xml,$(RENAMEDSRC))
planning:	$(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.cmark.html,$(RENAMEDSRC))
# planning:	$(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.md.xml,$(RENAMEDSRC))
