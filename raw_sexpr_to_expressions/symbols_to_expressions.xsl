<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

<xsl:include href="../subtransforms/ascii_hex_functions.xsl" />

<xsl:template match="ListSymbols">
  <Expressions>
    <xsl:apply-templates select="node()|@*"/>
  </Expressions>
</xsl:template>
  
<xsl:template match="List[not(*)]">
  <Reserved value="()" />
</xsl:template>

<xsl:template match="List[(*)]">
  <!-- This will mostly work - removes ? and similar from element names.
       Elements also can't start with [0-9] or [Xx][Mm][Ll] which might need
       to be escaped here as well
  -->
  
  <xsl:variable name="ecar">
    <xsl:call-template name="ascii-to-mixed">
      <xsl:with-param name="str" select="*[1]"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:element name="{$ecar}">
    <xsl:apply-templates select="*[position() > 1]" />
  </xsl:element>
</xsl:template>

<xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
</xsl:template>

</xsl:transform>
