<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >
<xsl:output method="xml" indent="yes"/>

<xsl:template match="/Language">
  <TokenTree>
    <xsl:apply-templates select="node()|@*"/>
  </TokenTree>
</xsl:template>

<xsl:variable name="LangName" >
  <xsl:value-of select="/Language/LanguageName"></xsl:value-of>
</xsl:variable>

<xsl:template match="Productions">
  <Header>
    <Include value ='#include "{$LangName}.production.h"' />
    <NL hexvalue="0a" />
    <Include value ='#include "{$LangName}.declarations.h"' />
    <NL hexvalue="0a" />
  </Header>
  <xsl:apply-templates select="AssignProduction"/>
</xsl:template>

<xsl:template match="AssignProduction">
  <NL hexvalue="0a" />
  <Label value="// AssignProduction {@label} -&gt; {$LangName}_grouping_{@grouping}" />
  <NL hexvalue="0a" />
  <ResultType value = "ptree {$LangName}_assign_production_{@label}(ptree_context ctx" />
  <xsl:apply-templates select="Grouping|Token" mode="ProductionFormals"/>
  <CL value=")" />
  <NL hexvalue="0a" />
  <OB value="{{" />
  <NL hexvalue="0a" />
  <xsl:apply-templates select="Grouping|Token" mode="AssignProductionArg"/>
  <R value="  return R;" />
  <NL hexvalue="0a" />
  <CB value="}}" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Grouping" mode="ProductionFormals">
  <COMMA value=", " />
  <GroupingProduction value="ptree /*{@type}*/ x{position()}" />
</xsl:template>

<xsl:template match="Token" mode="ProductionFormals">
  <COMMA value=", " />
  <TokenProduction value="token /*{@name}*/ x{position()}" />
</xsl:template>

<xsl:template match="Grouping" mode="AssignProductionArg">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  ptree R = x{position()};" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="AssignProductionArg">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  ptree R = parser_{$LangName}_ptree_from_token(ctx, {$LangName}_token_{@name}, x{position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="@*">
<xsl:copy>
  <xsl:apply-templates select="@*"/>
</xsl:copy>
</xsl:template>

<xsl:template match="node()">
</xsl:template>

</xsl:transform>
