<?xml version="1.0" encoding="UTF-8"?>
<!-- name some extensions xsltproc knows about -->
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >
  <xsl:output method="xml" indent="yes"/>
  
  <!-- Change the root node from tree to list -->
  <xsl:template match="/TokenTree">
    <TokenList>
      <xsl:apply-templates select="node()|@*"/>    
    </TokenList>
  </xsl:template>
  
  <!-- Copy attributes to the output unchanged -->
  <xsl:template match="@*">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
    </xsl:copy>
  </xsl:template>
  
  <!-- Transform elements on the way through -->
  <xsl:template match="node()">
    
    <!-- If there's no attributes, it's part of the tree structure we're flattening -->
    <xsl:if test="not(@*)">
      <xsl:apply-templates select="node()|@*"/>
    </xsl:if>
    
    <!-- If it does have attributes, leave it alone -->
    <xsl:if test="@*">
      <xsl:copy>
        <xsl:apply-templates select="node()|@*"/>
      </xsl:copy>
    </xsl:if>
  </xsl:template>    
</xsl:transform>
