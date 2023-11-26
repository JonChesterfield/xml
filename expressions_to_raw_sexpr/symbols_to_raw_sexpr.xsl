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
  <RawText>
    <xsl:apply-templates select="node()|@*"/>
  </RawText>
</xsl:template>

<!-- Default to copying everything -->
<xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
</xsl:template>


<xsl:template match="List">
  <xsl:text>(</xsl:text>
       <xsl:apply-templates select="node()|@*"/>
     <xsl:text>)</xsl:text>
</xsl:template>

<xsl:template match="Symbol">
  <xsl:variable name="depth" select="count(ancestor::*) - 1" />
  <xsl:if test="position()>1" >
    <xsl:text>&#xA;</xsl:text>
    <xsl:call-template name="spaces">
      <xsl:with-param name="n" select="2 * $depth"/>
    </xsl:call-template>
  </xsl:if>
  <xsl:value-of select="." />
</xsl:template>

<xsl:template name="spaces">
  <xsl:param name="n"/>
  <xsl:if test="$n > 0">
    <xsl:call-template name="spaces">
      <xsl:with-param name="n" select="$n - 1"/>
    </xsl:call-template>
    <xsl:text> </xsl:text>
  </xsl:if>
</xsl:template>


</xsl:transform>

