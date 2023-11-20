#!/bin/bash
set -e
set -x

make

SRC=text.txt

RawText=text.raw.xml
ListText=text.list.xml
ListSymbolText=text.listsymbol.xml
OutText=text.out

# check the style sheets
xmllint --relaxng xslt.rng *.xsl --noout


cat <<'EOF' > $RawText
<?xml version="1.0" encoding="UTF-8"?>
<!-- xsi:noNamespaceSchemaLocation="raw_to_list.xsd" -->
<RawText xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         >
<![CDATA[
EOF


cat $SRC >> $RawText
echo ']]></RawText>' >> $RawText


xmllint --relaxng raw_to_list.rng $RawText --noout
xsltproc --output $ListText raw_to_list.xsl $RawText

xmllint --relaxng list.rng $ListText --noout
xsltproc --output $ListSymbolText list_to_symbols.xsl $ListText

xmllint --relaxng list_to_symbols.rng $ListSymbolText --noout

# This is doing dubious things with newline
#  cat $ListSymbolText | xsltproc --output $ListSymbolText pretty.xsl -

xsltproc --output $OutText symbols_to_text.xsl $ListSymbolText
