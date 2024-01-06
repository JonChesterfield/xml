
planning_src := Planning
WORKDIR := .$(planning_src)/tmp


# Most of the complexity here is about spaces in filenames. Could escape more aggressively.
# Careful handling of spaces in filenames. Makefile uses whitespace delimited lists in most places.
# First, find all the files and replace space with a character makefile considers less special

SPACE_ESC := +
SPACE_ESCAPED_SRC := $(shell find "$(planning_src)" -type f -name "*md" | sed 's/ /$(SPACE_ESC)/g')

# All the directories other than the root
SPACE_ESCAPED_DIRS := $(filter $(planning_src)/%,$(shell find "$(planning_src)" -type d | sed 's/ /$(SPACE_ESC)/g'))

# Second, turn that back into a list of real filenames, where the spaces are escaped
# Can't do very much with that list but it can be used as a dependency on other targets
RENAMED_ESCAPED_SRC := $(patsubst $(planning_src)/%,$(WORKDIR)/%,$(SPACE_ESCAPED_SRC))
RENAMED_ESCAPED_DIRS := $(patsubst $(planning_src)/%,$(WORKDIR)/%,$(SPACE_ESCAPED_DIRS))

$(WORKDIR):
	@mkdir -p $(WORKDIR)


# Seems to be difficult to rely on a single file containing spaces and easy to depend on all of them
ALL_SRC_WITH_SPACES := $(subst $(SPACE_ESC),\ ,$(SPACE_ESCAPED_SRC))
$(RENAMED_ESCAPED_SRC):	$(WORKDIR)/%.md:	$(ALL_SRC_WITH_SPACES) | $(WORKDIR)
#	Thus retrieve the specific element in ALL_SRC_WITH_SPACES that is of interest
	@mkdir -p $(@D) # todo, fix this
	$(eval bs_spaces := $(planning_src)/$(subst $(SPACE_ESC),\ ,$*.md))
	@cmp $(bs_spaces) $@ >/dev/null 2>&1 || cp $(bs_spaces) $@


clean::
	rm -rf $(WORKDIR)

planning:	$(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.cmark.xml,$(RENAMED_ESCAPED_SRC))
planning:	$(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.cmark.html,$(RENAMED_ESCAPED_SRC))
#planning:	$(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.html,$(RENAMED_ESCAPED_SRC))
# planning:	$(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.md.xml,$(RENAMED_ESCAPED_SRC))


%.xml.card:	%.cmark.html
#	@echo $*
	$(eval stem := $(patsubst $(WORKDIR)/%,%,$(subst $(SPACE_ESC), ,$*)))
	@echo '<card name="$(stem)">' >> $@
	@cat $< >> $@
	@echo '</card>' >> $@

DERIVED_CARDS := $(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.xml.card,$(RENAMED_ESCAPED_SRC))
planning:	$(DERIVED_CARDS)

$(info "renamed escaped src")
$(info $(RENAMED_ESCAPED_SRC))

$(info "renamed escaped dirs")
$(info $(RENAMED_ESCAPED_DIRS))

$(info "derived cards")
$(info $(DERIVED_CARDS))

# can determine the files under the scope of a particular directory
$(RENAMED_ESCAPED_DIRS:=.cards.xml):	$(WORKDIR)/%.cards.xml:	$(DERIVED_CARDS)
	@mkdir -p $(@D)
#	echo $* > $@
#	echo $(filter $(WORKDIR)/$*/%,$(RENAMED_ESCAPED_SRC))
	echo '<?xml version="1.0" encoding="UTF-8"?>' > $@
	$(eval stem := $(patsubst $(WORKDIR)/%,%,$(subst $(SPACE_ESC), ,$*)))
	@echo '<cards name="$(stem)">' >> $@
	cat $(DERIVED_CARDS) >> $@
	@echo '</cards>' >> $@


planning: $(RENAMED_ESCAPED_DIRS:=.cards.xml)
