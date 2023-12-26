<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

<xsl:output method="xml" indent="no"/>
<xsl:strip-space elements="*"/>

<xsl:template match="/HexTokenList">
  <RawBinary>
    <xsl:apply-templates select="node()|@*"/>
  </RawBinary>
</xsl:template>

<xsl:template match="@hexvalue|@hexliteral" >
  <xsl:value-of select="." />
</xsl:template>

<!-- Drop any other attributes that got this far -->
<xsl:template match="@*" />

<!-- Drop the node itself, recur on the contents -->
<xsl:template match="node()">
  <xsl:apply-templates select="node()|@*"/>
</xsl:template>

</xsl:transform>
