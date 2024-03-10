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
    <Include value ='#include "{$LangName}.productions.h"' />
    <NL hexvalue="0a" />
    <Include value ='#include "{$LangName}.declarations.h"' />
    <NL hexvalue="0a" />
    <Include value ='#include "{$LangName}.ptree.h"' />
    <NL hexvalue="0a" />
  </Header>
  <xsl:apply-templates select="ListProduction"/>
</xsl:template>

<xsl:template match="ListProduction">
  <NL hexvalue="0a" />
  <Label value="// ListProduction {@label} -&gt; {$LangName}_grouping_{@grouping}" />
  <NL hexvalue="0a" />
  <ResultType value = "ptree {$LangName}_list_production_{@label}(ptree_context ctx" />
  <xsl:apply-templates select="Grouping|Token" mode="ProductionFormals"/>
  <CL value=")" />
  <NL hexvalue="0a" />
  <OB value="{{" />
  <NL hexvalue="0a" />

  <ES value = "  enum {{" />
  <NL hexvalue="0a" />
  <T value="    argument_arity = {count(Grouping[@position])}," />
  <NL hexvalue="0a" />
  <T value="    token_arity = {count(Token[@position])}," />
  <NL hexvalue="0a" />
  <T value="    total_arity = argument_arity + token_arity," />
  <NL hexvalue="0a" />
  <EE value = "  }};" />
  <NL hexvalue="0a" />

  <ResTmp value = '  ptree R = {$LangName}_ptree_expression_create_uninitialised(ctx, {$LangName}_grouping_{@grouping}, total_arity);' />
  <NL hexvalue="0a" />

  <xsl:apply-templates select="Grouping|Token" mode="ListProductionArg"/>
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

<xsl:template match="Grouping" mode="ListProductionArg">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  {$LangName}_ptree_expression_initialise_element(R, {@position}-1, x{position()});" />    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="ListProductionArg">
  <xsl:choose>
    <xsl:when test="@position">
      <T value = "  ptree tmp{position()} = parser_{$LangName}_ptree_from_token({$LangName}_ptree_context, {$LangName}_token_{@name}, x{position()});" />
      <NL hexvalue="0a" />
      <P value="  {$LangName}_ptree_expression_initialise_element(R, {@position}-1, tmp{position()});" />
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
