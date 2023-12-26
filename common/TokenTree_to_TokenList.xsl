<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

<xsl:output method="xml" indent="yes"/>

<xsl:template match="/TokenTree">
  <TokenList>
    <xsl:apply-templates select="node()|@*"/>    
  </TokenList>
</xsl:template>

<xsl:template match="@*">
        <xsl:copy>
       <xsl:apply-templates select="@*"/>
    </xsl:copy>
</xsl:template>

<xsl:template match="node()">
  <xsl:if test="not(@*)">

    <xsl:apply-templates select="node()|@*"/>

  </xsl:if>
  <xsl:if test="@*">
    <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:if>
</xsl:template>



</xsl:transform>
