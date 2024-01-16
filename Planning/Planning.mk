obsidian_dir := ${HOME}/Documents/Obsidian
planning_src := ${obsidian_dir}/PlanningWIP
output_dir := ${obsidian_dir}/Projects
WORKDIR := /tmp/.ObsidianXML

# Considering how to adapt to multiple projects
# Distinct workdirs has some attraction. Ideally wouldn't require a common directory root.

# Most of the complexity here is about spaces in filenames. Could escape more aggressively.
# First, find all the files and replace space with a character makefile considers less special
# That's also very directory-independent
# find understands multiple directories, find foo bar -type f will work
# might be able to work in terms of src with a common prefix instead of filtering out
# the root

# Currently this changes the prefix from some source tree to a working tree, might be
# simpler to prefix with a fixed workdir which is subtracted at the corresponding places

SPACE_ESC := +
SPACE_ESCAPED_SRC := $(shell find "$(planning_src)" -type f -name "*md" | sed 's/ /$(SPACE_ESC)/g')

# All the directories other than the root
SPACE_ESCAPED_DIRS_INCLUDING_ROOT := $(shell find "$(planning_src)" -type d | sed 's/ /$(SPACE_ESC)/g')
SPACE_ESCAPED_DIRS := $(filter $(planning_src)/%,$(SPACE_ESCAPED_DIRS_INCLUDING_ROOT))

# Get names in some corresponding work directory
WORKDIR_ESCAPED_SRC := $(patsubst $(planning_src)/%,$(WORKDIR)/%,$(SPACE_ESCAPED_SRC))
WORKDIR_ESCAPED_DIRS := $(patsubst $(planning_src)/%,$(WORKDIR)/%,$(SPACE_ESCAPED_DIRS))

$(WORKDIR):
	@mkdir -p $(WORKDIR)

clean::
	@rm -rf $(WORKDIR)

# $(info "for e. in" $(WORKDIR_ESCAPED_SRC))
# $(info "get $(WORKDIR)/%%.md")
# $(info "if a. change in" $(subst $(SPACE_ESC),\ ,$(SPACE_ESCAPED_SRC)))

# Seems to be difficult to rely on a single file containing spaces and easy to depend on all of them
ALL_SRC_WITH_SPACES := $(subst $(SPACE_ESC),\ ,$(SPACE_ESCAPED_SRC))
$(WORKDIR_ESCAPED_SRC):	$(WORKDIR)/%.md:	$(ALL_SRC_WITH_SPACES) | $(WORKDIR)
#	Thus retrieve the specific element in ALL_SRC_WITH_SPACES that is of interest
#	and copy it to the renamed equivalent in workdir iff newer
	@mkdir -p $(@D) # todo, fix this
	$(eval bs_spaces_src := $(planning_src)/$(subst $(SPACE_ESC),\ ,$*.md))
	@cmp $(bs_spaces_src) $@ >/dev/null 2>&1 || cp $(bs_spaces_src) $@

# %.md -> %.cmark.html is a known pattern in the root so can depend directly on the cmark.html
DERIVED_CARDS := $(patsubst $(WORKDIR)/%.md,$(WORKDIR)/%.xml.task,$(WORKDIR_ESCAPED_SRC))
$(DERIVED_CARDS):	%.xml.task:	%.cmark.html
#	Trims the workdir and retrieves spaces to get the original file name
	$(eval stem := $(patsubst $(WORKDIR)/%,%,$(subst $(SPACE_ESC), ,$*)))
	@echo '<task name="$(notdir $(stem))">' > $@
	@cat $< >> $@
	@echo '</task>' >> $@


COLLECTED_TASKS := $(WORKDIR_ESCAPED_DIRS:=.card.xml)
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
	@cat /dev/null $(sort $(filter $(WORKDIR)/$*/%,$(DERIVED_CARDS))) >> $@
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

.PHONY:	publish
publish:	Planning.html
#	This is a hack. Renaming files in the obsidian dir don't reliably trigger
# 	a rebuild of planning - something is wrong in the dependency graph
#	or using time based rebuild across multiple systems is a bad thing
#	Deleting the work dir after the build makes the next scheduled run correct
	@cmp Planning.html ${output_dir}/Planning.html >/dev/null 2>&1 || cp Planning.html ${output_dir}/Planning.html
	@rm -rf $(WORKDIR) Planning.html

clean::
	@rm -f Planning.html

planning: Planning.html

