<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

<xsl:output method="xml" indent="yes"/>
<xsl:strip-space elements="*"/>

<!-- Default to copying everything -->
<xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
</xsl:template>

<xsl:template match="ListText">
<ListSymbols>    
  <xsl:apply-templates select="node()|@*"/>
</ListSymbols>
</xsl:template>

<xsl:template match="List">
<List>    
  <xsl:apply-templates select="node()|@*"/>    
</List>
</xsl:template>

<xsl:template match="text()">
  <xsl:for-each select="str:tokenize(normalize-space(.),' ')" >
    <Symbol><xsl:value-of select="normalize-space(.)" /></Symbol>
  </xsl:for-each>
</xsl:template>

</xsl:transform>
  
  
