<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

<xsl:template match="/Expressions">
      <ListSymbols>
        <xsl:apply-templates select="node()|@*"/>
      </ListSymbols>
</xsl:template>
  
<xsl:template match="Reserved" >
  <xsl:if test="@value = '()'" >
    <List />
  </xsl:if>
</xsl:template>

<xsl:template match="Symbol" >
<Symbol>
  <xsl:value-of select="." />
</Symbol>
</xsl:template>

<xsl:template match="text()" />
<xsl:template match="@*" />

<xsl:template name="recurse" match="node()">
  <!-- The test avoids spurious () turning up in the output -->
  <!-- Not sure what node is present but has no children -->
  <xsl:if test="normalize-space(local-name(.))" >
    <List>
      <Symbol>
        <xsl:value-of select="local-name(.)" />
      </Symbol>
      <xsl:for-each select="node()" >
        <xsl:apply-templates select="."/>
      </xsl:for-each>
    </List>
  </xsl:if>
</xsl:template>



</xsl:transform>
