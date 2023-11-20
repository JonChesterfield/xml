<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               extension-element-prefixes="str ext"
               >

<xsl:output method="xml" indent="yes"/>
<xsl:strip-space elements="*"/>

<xsl:template match="/">
  <Derived>
    <Symbols>
      <!-- Sorting these probably involves ext:node-set -->
      <xsl:apply-templates select="node()|@*" mode="extract"/>
    </Symbols>
    <Textual>
      <xsl:apply-templates select="node()|@*" mode="Textual"/>
    </Textual>
    <xsl:apply-templates select="node()|@*"/>
  </Derived>
</xsl:template>

<!-- Default to copying everything -->
<xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
</xsl:template>



<xsl:template match="node()|@*" mode="extract">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*" mode="extract"/>
     </xsl:copy>
</xsl:template>

<xsl:template match="ListSymbols" mode="extract">
  <xsl:apply-templates select="node()|@*" mode="extract"/>
</xsl:template>

<xsl:template match="List" mode="extract">
  <xsl:apply-templates select="node()|@*" mode="extract"/>
</xsl:template>


<xsl:template match="node()|@*" mode="Textual">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*" mode="Textual"/>
     </xsl:copy>
</xsl:template>

<xsl:template match="ListSymbols" mode="Textual">
  <xsl:apply-templates select="node()|@*" mode="Textual"/>
</xsl:template>

<xsl:template match="List" mode="Textual">
(<xsl:apply-templates select="node()|@*" mode="Textual"/>)
</xsl:template>
<xsl:template match="Symbol" mode="Textual">
  <xsl:value-of select="." />
</xsl:template>


</xsl:transform>
