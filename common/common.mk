common/preconditions: $(call get_schema_name, %common/TokenTree.rng)
common/preconditions: $(call get_schema_name, %common/TokenList.rng)
common/preconditions: $(call get_schema_name, %common/HexTokenList.rng)
common/preconditions: $(call get_schema_name, %common/RawBinary.rng)
common/preconditions: subtransforms/pretty.xsl
common/preconditions: $(xmllint) $(xsltproc)

# TokenTree to TokenList conversion, with pretty printing where that doesn't segv
validate/TokenTree_to_TokenList: common/TokenTree_to_TokenList.xsl | common/preconditions
	@$(INTERPRETER)$(xmllint) --relaxng xslt.rng "$<" $(XMLLINTOPTS) --quiet

%.TokenList.prelint.xml:	%.TokenTree.xml common/TokenTree_to_TokenList.xsl | validate/TokenTree_to_TokenList
	@$(INTERPRETER)$(xmllint) --relaxng $(call get_schema_name, %common/TokenTree.rng) "$<" $(XMLLINTOPTS) --quiet
	@$(INTERPRETER)$(xsltproc) $(XSLTPROCOPTS) --output "$@" common/TokenTree_to_TokenList.xsl "$<"

%.TokenList.pretty.xml:	%.TokenList.prelint.xml | common/preconditions 
	@$(INTERPRETER)$(xsltproc) $(XSLTPROCOPTS) --output "$@" subtransforms/pretty.xsl "$<" || cp "$<" "$@"

%.TokenList.xml:	%.TokenList.pretty.xml | common/preconditions 
	@$(INTERPRETER)$(xmllint) --relaxng $(call get_schema_name, %common/TokenList.rng) "$<" $(XMLLINTOPTS) --quiet
	@cp "$<" "$@"

# TokenList to HexTokenList conversion
validate/TokenList_to_HexTokenList: common/TokenList_to_HexTokenList.xsl | common/preconditions 
	@$(INTERPRETER)$(xmllint) --relaxng xslt.rng "$<" $(XMLLINTOPTS) --quiet

%.HexTokenList.prelint.xml:	%.TokenList.xml common/TokenList_to_HexTokenList.xsl | validate/TokenList_to_HexTokenList
	@$(INTERPRETER)$(xmllint) --relaxng $(call get_schema_name, %common/TokenList.rng) "$<" $(XMLLINTOPTS) --quiet
	@$(INTERPRETER)$(xsltproc) $(XSLTPROCOPTS) --output "$@" common/TokenList_to_HexTokenList.xsl "$<"

%.HexTokenList.pretty.xml:	%.HexTokenList.prelint.xml | common/preconditions 
	@$(INTERPRETER)$(xsltproc) $(XSLTPROCOPTS) --output "$@" subtransforms/pretty.xsl "$<" || cp "$<" "$@"

%.HexTokenList.xml:	%.HexTokenList.pretty.xml | common/preconditions 
	@$(INTERPRETER)$(xmllint) --relaxng $(call get_schema_name, %common/HexTokenList.rng) "$<" $(XMLLINTOPTS) --quiet
	@cp "$<" "$@"


# HexTokenList to RawBinary conversion
validate/HexTokenList_to_RawBinary: common/HexTokenList_to_RawBinary.xsl | common/preconditions 
	@$(INTERPRETER)$(xmllint) --relaxng xslt.rng "$<" $(XMLLINTOPTS) --quiet

%.RawBinary.prelint.xml:	%.HexTokenList.xml common/HexTokenList_to_RawBinary.xsl | validate/HexTokenList_to_RawBinary
	@$(INTERPRETER)$(xmllint) --relaxng $(call get_schema_name, %common/HexTokenList.rng) "$<" $(XMLLINTOPTS) --quiet
	@$(INTERPRETER)$(xsltproc) $(XSLTPROCOPTS) --output "$@" common/HexTokenList_to_RawBinary.xsl "$<"

%.RawBinary.pretty.xml:	%.RawBinary.prelint.xml | common/preconditions 
	@$(INTERPRETER)$(xsltproc) $(XSLTPROCOPTS) --output "$@" subtransforms/pretty.xsl "$<" || cp "$<" "$@"

%.RawBinary.xml:	%.RawBinary.pretty.xml | common/preconditions 
	@$(INTERPRETER)$(xmllint) --relaxng $(call get_schema_name, %common/RawBinary.rng) "$<" $(XMLLINTOPTS) --quiet
	@cp "$<" "$@"
