obsidian_dir := ${HOME}/Documents/Obsidian
planning_src := ${obsidian_dir}/PlanningWIP
output_dir := ${obsidian_dir}/Projects
WORKDIR := /tmp/.ObsidianXML

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

%.xml.task:	%.cmark.html
	$(eval stem := $(patsubst $(WORKDIR)/%,%,$(subst $(SPACE_ESC), ,$*)))
	@echo '<task name="$(notdir $(stem))">' > $@
	@cat $< >> $@
	@echo '</task>' >> $@

DERIVED_CARDS := $(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.xml.task,$(RENAMED_ESCAPED_SRC))

COLLECTED_TASKS := $(RENAMED_ESCAPED_DIRS:=.card.xml)
# can determine the files under the scope of a particular directory
# depends all all derived cards, so makes all of them, and then pulls out those
# applicable to the directory identified by the stem
$(COLLECTED_TASKS):	$(WORKDIR)/%.card.xml:	$(DERIVED_CARDS)
	@mkdir -p $(@D)
#	echo '<?xml version="1.0" encoding="UTF-8"?>' > $@
	@echo '' > $@
	$(eval stem := $(patsubst $(WORKDIR)/%,%,$(subst $(SPACE_ESC), ,$*)))
	@echo '<card name="$(stem)">' >> $@
# cat hangs if the list of files is empty and dev null contributes nothing
	@cat /dev/null $(filter $(WORKDIR)/$*/%,$(DERIVED_CARDS)) >> $@
	@echo '</card>' >> $@

$(WORKDIR)/Planning.cards.xml:	$(COLLECTED_TASKS)
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $@
	@echo '<Project name="Planning">' >> $@
#	Sort isnt right here but it's predictable for now
	@cat $(sort $(COLLECTED_TASKS)) >> $@
	@echo '</Project>' >> $@

Planning.html:	$(WORKDIR)/Planning.cards.xml card_to_html.xsl
	@xsltproc $(XSLTPROCOPTS) --output "$@" card_to_html.xsl "$<"

# Only copies changed files to not retrigger sync
${output_dir}/Planning.html:	Planning.html
	@echo "Recreating $@"
	@cmp Planning.html $@ >/dev/null 2>&1 || cp Planning.html $@

publish::	${output_dir}/Planning.html


clean::
	rm -f Planning.html

planning: Planning.html
