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

<xsl:template match="/IndentedSexpr">
  <RawText>
    <xsl:apply-templates select="node()|@*"/>
  </RawText>
</xsl:template>

<xsl:template match="List">
  <xsl:text>(</xsl:text>
  <xsl:apply-templates select="node()|@*"/>
  <xsl:text>)</xsl:text>
</xsl:template>

<xsl:template match="Newline">
  <xsl:text>&#xA;</xsl:text>
</xsl:template>

<xsl:template match="Spaces">
    <xsl:call-template name="spaces">
      <xsl:with-param name="n" select="2 * @value"/>
    </xsl:call-template>
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

<xsl:template match="Symbol">

  <xsl:variable name="esc4" select="str:replace(., '_2D','-')" />
  <xsl:variable name="esc3" select="str:replace($esc4, '_26','&amp;')" />
  <xsl:variable name="esc2" select="str:replace($esc3, '_3E','&gt;')" />
  <xsl:variable name="esc1" select="str:replace($esc2, '_3C','&lt;')" />
  <xsl:variable name="esc0" select="str:replace($esc1, '_5F','_')" />

  <xsl:value-of select="$esc0"/>
</xsl:template>


</xsl:transform>

