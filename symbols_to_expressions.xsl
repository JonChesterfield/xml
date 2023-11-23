<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >
<!-- <xsl:output method="xml" indent="yes"/> -->


<xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
</xsl:template>

<xsl:template match="ListSymbols">
<Expressions>
  <xsl:apply-templates select="node()|@*"/>
</Expressions>
</xsl:template>
  
<xsl:template match="List[not(*)]">
  <NilList />
</xsl:template>

<xsl:template match="List[(*)]">
  <xsl:variable name="car" select="*[1]" />

  <xsl:element name="{$car}">
    <xsl:apply-templates select="*[position() > 1]" />
</xsl:element>

</xsl:template>

</xsl:transform>
