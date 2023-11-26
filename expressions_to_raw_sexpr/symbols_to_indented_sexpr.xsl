<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

<xsl:output method="xml" indent="yes"/>

<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="xsl:text"/>

<xsl:template match="/ListSymbols">
  <IndentedSexpr>
    <xsl:apply-templates select="node()|@*"/>
  </IndentedSexpr>
</xsl:template>

<xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
</xsl:template>

<xsl:template match="List">
  <List>
    <xsl:apply-templates select="node()|@*"/>
  </List>
</xsl:template>

<xsl:template match="Symbol">
  <xsl:variable name="depth" select="count(ancestor::*) - 1" />
  <xsl:if test="position()>1" >
    <Newline/>
    <Spaces value="{2 * $depth}" />
  </xsl:if>
  <Symbol>
    <xsl:value-of select="." />
  </Symbol>
</xsl:template>

</xsl:transform>

