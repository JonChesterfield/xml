<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

  <xsl:output method="xml" indent="yes"/>
  <xsl:strip-space elements="*"/>

  <xsl:template match="/CTree">
    <CSyntax>
      <xsl:apply-templates select="node()|@*"/>
    </CSyntax>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>


  <xsl:template match="Parameters">
    <Parameters>
      <Punctuator value="("/>
      <xsl:apply-templates select="node()|@*"/>
      <Punctuator value=")"/>
    </Parameters>
  </xsl:template>

  <xsl:template match="Parameter">
      <xsl:if test="position()>1">
        <Punctuator value=","/>
      </xsl:if>
    <Parameter>
      <xsl:apply-templates select="node()|@*"/>
    </Parameter>
  </xsl:template>

  <xsl:template match="Block">
    <Block>
      <Punctuator value="{{"/>
      <xsl:apply-templates select="node()|@*"/>
      <Punctuator value="}}"/>
    </Block>
  </xsl:template>

</xsl:transform>
