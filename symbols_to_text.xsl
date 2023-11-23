<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               
>

<!-- Takes a root RawText and hacks with the parens, emitting text -->

<xsl:output method="text"/>
<xsl:strip-space elements="List ListSymbols"/>

<xsl:template match="ListSymbols">
  <xsl:apply-templates select="node()|@*"/>
</xsl:template>

<xsl:template match="List">
<text>(</text>
  <xsl:apply-templates select="node()|@*"/>
<text>)</text>
</xsl:template>

<xsl:template match="Symbol">
<text><xsl:value-of select="."/></text>

</xsl:template>

</xsl:transform>
  
  
