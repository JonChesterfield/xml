<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

<xsl:output method="xml" indent="yes"/>
<xsl:strip-space elements="*"/>


<xsl:include href="../subtransforms/ascii_hex_functions.xsl" />

<xsl:template match="/TokenList">
  <HexTokenList>
    <xsl:apply-templates select="node()"/>
  </HexTokenList>
</xsl:template>

<xsl:template match="node()|@*">
  <xsl:copy>
    <xsl:apply-templates select="node()|@*"/>
  </xsl:copy>
</xsl:template>

<!-- Name is optional in tokenlist, could discard or preserve here -->
<xsl:template match="@name">
  <xsl:copy-of select="." />
</xsl:template>

<!-- Already hex, pass through -->
<xsl:template match="@hexliteral|@hexvalue" >
  <xsl:copy-of select="." />
</xsl:template>

<!-- Convert literal and value to hex versions -->
<xsl:template match="@literal" >
  <xsl:attribute name="hexliteral">
    <xsl:call-template name="ascii-to-hex">
      <xsl:with-param name="str" select="."/>
    </xsl:call-template>
  </xsl:attribute>
</xsl:template>

<xsl:template match="@value" >
  <xsl:attribute name="hexvalue">
    <xsl:call-template name="ascii-to-hex">
      <xsl:with-param name="str" select="."/>
    </xsl:call-template>
  </xsl:attribute>
</xsl:template>


</xsl:transform>
