<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
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
    </Symbols>
    <Textual>
      <xsl:apply-templates select="node()|@*" mode="Textual"/>
      <xsl:text>&#xA;</xsl:text>
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

  <xsl:text>&#xA;</xsl:text>
  <xsl:call-template name="spaces">
    <xsl:with-param name="n" select="2 * $depth"/>
  </xsl:call-template>

  <xsl:text>(</xsl:text>

  <xsl:apply-templates select="node()" mode="Textual"/>
  <xsl:apply-templates select="@*" mode="Textual"/>

  <xsl:text>)</xsl:text>

</xsl:template>

<xsl:template match="Symbol" mode="Textual">
  <xsl:variable name="depth" select="count(ancestor::*) - 1" />
  <xsl:if test="position()>1" >
<!--
    <S>
      <xsl:attribute name="depth">    
        <xsl:value-of select="$depth"/>
      </xsl:attribute> 
    </S>
-->
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
