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
    <CPunctuation>
      <xsl:apply-templates select="node()|@*"/>
    </CPunctuation>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="ReturnType">
    <ReturnType>
      <xsl:value-of select="." />        
    </ReturnType>
    <Punctuator value=" "/>
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
        <Punctuator value=", "/>
      </xsl:if>

    <Parameter>
      <ParameterType>
        <xsl:value-of select="*[1]" />
      </ParameterType>

      <xsl:if test="count(*) = 2" >
        <Punctuator value=" "/>
        <ParameterName>
          <xsl:value-of select="*[2]" />
        </ParameterName>        
      </xsl:if>

    </Parameter>
  </xsl:template>

  <xsl:template match="Block">
    <Block>
      <Punctuator value="{{"/>
      <Punctuator value="&#xA;"/>
      <xsl:apply-templates select="node()|@*"/>
      <Punctuator value="}}"/>
      <Punctuator value="&#xA;"/>
    </Block>
  </xsl:template>

  <xsl:template match="Statement|ReturnStatement">
      <xsl:call-template name="emit-spaces"/>

    <xsl:element name="{local-name()}" >
      <xsl:value-of select="." />        
    </xsl:element>
    <Punctuator value="&#xA;"/>
  </xsl:template>

<xsl:template name="emit-spaces">
    <xsl:variable name="depth" select="count(ancestor::*) - 2" />
    <xsl:if test="$depth > 0" >
      <xsl:call-template name="recur-spaces">
        <xsl:with-param name="n" select="$depth"/>
      </xsl:call-template>
    </xsl:if>
</xsl:template>

<xsl:template name="recur-spaces">
  <!-- Could use choose to fold some of the punctuators together -->
  <xsl:param name="n"/>
  <xsl:if test="$n > 0">
    <xsl:call-template name="recur-spaces">
      <xsl:with-param name="n" select="$n - 1"/>
    </xsl:call-template>
    <Punctuator value="  "/>
  </xsl:if>
</xsl:template>

</xsl:transform>
