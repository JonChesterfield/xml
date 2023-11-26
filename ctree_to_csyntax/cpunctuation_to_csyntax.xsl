<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >

  <xsl:output method="xml" />
  <xsl:strip-space elements="*"/>

  <xsl:template match="/CPunctuation">
    <CSyntax>
      <xsl:apply-templates select="node()|@*"/>
    </CSyntax>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="Punctuator" >
   <xsl:value-of select="@value" />
  </xsl:template>

  <xsl:template match="ReturnType">
    <xsl:value-of select="." />        
  </xsl:template>

  <xsl:template match="Statement">
    <xsl:value-of select="." />
    <xsl:text>;</xsl:text>
  </xsl:template>

  <xsl:template match="ReturnStatement">
    <xsl:text>return </xsl:text>
    <xsl:value-of select="." />
    <xsl:text>;</xsl:text>
  </xsl:template>

  <xsl:template match="ParameterType">
    <xsl:value-of select="." />
  </xsl:template>

  <xsl:template match="ParameterName">
    <xsl:value-of select="." />
  </xsl:template>

  <xsl:template match="Name">
    <xsl:value-of select="." />        
  </xsl:template>


  <xsl:template match="Parameters">
    <xsl:apply-templates select="node()|@*"/>
  </xsl:template>

  <xsl:template match="Parameter">
    <xsl:apply-templates select="node()|@*"/>
  </xsl:template>

  <xsl:template match="Block">
    <xsl:apply-templates select="node()|@*"/>
  </xsl:template>

  <xsl:template match="Function">
    <xsl:apply-templates select="node()|@*"/>
  </xsl:template>

</xsl:transform>
