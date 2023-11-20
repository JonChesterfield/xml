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
      <!-- Sorting these probably involves ext:node-set, not clear what
           is deleting the symbol annotation so currently recreating it
      -->
      
      <xsl:variable name="Symbols">
        <xsl:apply-templates select="node()|@*" mode="extract"/>
      </xsl:variable>

      <xsl:variable name="SortedSymbols">
        <xsl:for-each select="ext:node-set($Symbols)/*" >
          <xsl:sort/>
          <Symbol><xsl:value-of select="." /></Symbol>
        </xsl:for-each>
      </xsl:variable>

      <!-- not preceding seems to be OK here, distinct-values is a xlst 2 thing -->
      <xsl:for-each select="ext:node-set($SortedSymbols)/*[not(.=preceding::*)]" >
        <Symbol><xsl:value-of select="." /></Symbol>
      </xsl:for-each>
      
      <!-- <xsl:apply-templates select="node()|@*" mode="extract"/> -->
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

<xsl:template match="Symbol" mode="extract">
  <Symbol>
    <xsl:value-of select="." />
  </Symbol>
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
<xsl:variable name="depth" select="count(ancestor::*) - 1" />
<L>
  <xsl:attribute name="depth">    
    <xsl:value-of select="$depth"/>
  </xsl:attribute>
</L>
<xsl:apply-templates select="node()" mode="Textual"/>
<xsl:apply-templates select="@*" mode="Textual"/>
<R>
  <xsl:attribute name="depth">    
    <xsl:value-of select="$depth"/>
  </xsl:attribute>
</R>
</xsl:template>

<xsl:template match="Symbol" mode="Textual">
  <xsl:if test="position()>1" >
    <S/>
  </xsl:if>
  <xsl:value-of select="." />
</xsl:template>


</xsl:transform>
