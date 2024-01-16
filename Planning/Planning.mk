obsidian_dir := ${HOME}/Documents/Obsidian

output_dir := ${obsidian_dir}/Projects

WORKDIR := /tmp/.ObsidianXML

# TODO: Read these from somewhere under obsidian_dir
project_names := PlanningWIP AMD

planning_src := $(addprefix ${obsidian_dir}/,${project_names})

# Considering how to adapt to multiple projects
# Distinct workdirs has some attraction. Ideally wouldn't require a common directory root.

# Most of the complexity here is about spaces in filenames. Could escape more aggressively.
# First, find all the files and replace space with a character makefile considers less special
# That's also very directory-independent
# find understands multiple directories, find foo bar -type f will work
# might be able to work in terms of src with a common prefix instead of filtering out
# the root

SPACE_ESC := +
SPACE_ESCAPED_SRC := $(shell find $(planning_src) -type f -name "*md" | sed 's/ /$(SPACE_ESC)/g')

# Want all the directories other than the roots
SPACE_ESCAPED_DIRS := $(shell find $(planning_src) -mindepth 1 -type d | sed 's/ /$(SPACE_ESC)/g')

# Get names in some corresponding work directory
# Simpler to prefix than to substitute
WORKDIR_ESCAPED_SRC := $(addprefix $(WORKDIR),$(SPACE_ESCAPED_SRC))
WORKDIR_ESCAPED_DIRS := $(addprefix $(WORKDIR),$(SPACE_ESCAPED_DIRS))

$(WORKDIR):
	@mkdir -p $(WORKDIR)

clean::
	@rm -rf $(WORKDIR)

# Seems to be difficult to rely on a single file containing spaces and easy to depend on all of them
ALL_SRC_WITH_SPACES := $(subst $(SPACE_ESC),\ ,$(SPACE_ESCAPED_SRC))
$(WORKDIR_ESCAPED_SRC):	$(WORKDIR)%.md:	$(ALL_SRC_WITH_SPACES) | $(WORKDIR)
#	Thus retrieve the specific element in ALL_SRC_WITH_SPACES that is of interest
#	and copy it to the renamed equivalent in workdir iff newer
	@mkdir -p $(@D) # todo, fix this
	$(eval bs_spaces_src := $(subst $(SPACE_ESC),\ ,$*.md))
	@cmp $(bs_spaces_src) $@ >/dev/null 2>&1 || cp $(bs_spaces_src) $@

# %.md -> %.cmark.html is a known pattern in the root so can depend directly on the cmark.html
DERIVED_CARDS := $(WORKDIR_ESCAPED_SRC:.md=.xml.task)

$(DERIVED_CARDS):	%.xml.task:	%.cmark.html
	@echo '<task name="$(notdir $(subst $(SPACE_ESC), ,$*))">' > $@
	@cat $< >> $@
	@echo '</task>' >> $@

COLLECTED_TASKS := $(WORKDIR_ESCAPED_DIRS:=.card.xml)
# can determine the files under the scope of a particular directory
# depends all all derived cards, so makes all of them, and then pulls out those
# applicable to the directory identified by the stem
$(COLLECTED_TASKS):	$(WORKDIR)%.card.xml:	$(DERIVED_CARDS)
	@mkdir -p $(@D)
	@echo '' > $@
	@echo '<card name="$(notdir $(subst $(SPACE_ESC), ,$*))">' >> $@
# cat hangs if the list of files is empty and dev null contributes nothing
	@cat /dev/null $(sort $(filter $(WORKDIR)$*/%,$(DERIVED_CARDS))) >> $@
	@echo '</card>' >> $@

PROJECT_XML := $(addprefix $(WORKDIR)/,$(addsuffix .cards.xml,$(project_names)))
$(PROJECT_XML):	$(WORKDIR)/%.cards.xml:	$(COLLECTED_TASKS)
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $@
	@echo '<Project name="$*">' >> $@
	@cat /dev/null $(sort $(filter $(WORKDIR)${obsidian_dir}/$*/%,$(COLLECTED_TASKS))) >> $@
	@echo '</Project>' >> $@


PROJECT_HTML := $(addprefix $(WORKDIR)/,$(addsuffix .html,$(project_names)))
PUBLISH_HTML := $(addprefix ${output_dir}/,$(addsuffix .html,$(project_names)))

$(PROJECT_HTML):	%.html:	%.cards.xml card_to_html.xsl
	@xsltproc $(XSLTPROCOPTS) --output "$@" card_to_html.xsl "$<"

# Only copies changed files to not retrigger obsidian sync
$(PUBLISH_HTML):	${output_dir}/%.html:	$(WORKDIR)/%.html
	@cmp "$@" "$<" >/dev/null 2>&1 || cp "$<" "$@"

# TODO: Work out if the dependency graph is sound for this to isolate timestamps
# from errors in this makefile
projects: $(PUBLISH_HTML)

publish:	projects
#	This is a hack. Renaming files in the obsidian dir don't reliably trigger
# 	a rebuild of planning - something is wrong in the dependency graph
#	(or using time based rebuild across multiple systems is a bad thing)
#	Deleting the work dir after the build makes the next scheduled make correct
	@rm -rf $(WORKDIR)

clean::
	@rm -f $(PROJECT_HTML)

planning: Planning.html

