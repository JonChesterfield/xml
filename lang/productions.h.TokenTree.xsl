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

<xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'" />
<xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
<xsl:variable name="UpcaseLangName" >
  <xsl:value-of select="translate($LangName,$lowercase,$uppercase)" />
</xsl:variable>

<xsl:template match="Productions">
  <Header>
    <Header0 value ="#ifndef {$UpcaseLangName}_PRODUCTION_DECLARATIONS_H_INCLUDED" />
    <NL hexvalue="0a" />
    <Header1 value ="#define {$UpcaseLangName}_PRODUCTION_DECLARATIONS_H_INCLUDED" />
    <NL hexvalue="0a0a" />

    <Header value='#include "../tools/token.h"' />
    <NL hexvalue = "0a" />
    <Header value='#include "{$LangName}.ptree.h"' />
    <NL hexvalue = "0a" />
  </Header>

  <xsl:apply-templates select="ListProduction|AssignProduction"/>

  <NL hexvalue="0a0a" />
  <Footer>
    <Footer0 value="#endif /* {$UpcaseLangName}_LEXER_DECLARATIONS_H_INCLUDED */" />
  </Footer>
</xsl:template>

<xsl:template match="ListProduction">
  <NL hexvalue="0a" />
  <Label value="// ListProduction {@label} -&gt; {$LangName}_grouping_{@grouping}" />
  <NL hexvalue="0a" />
  <ResultType value = "ptree {$LangName}_list_production_{@label}(ptree_context ctx" />
  <xsl:apply-templates select="Grouping|Token" mode="ProductionFormals"/>
  <CL value=");" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="AssignProduction">
  <NL hexvalue="0a" />
  <Label value="// AssignProduction {@label} -&gt; {$LangName}_grouping_{@grouping}" />
  <NL hexvalue="0a" />
  <ResultType value = "ptree {$LangName}_assign_production_{@label}(ptree_context ctx" />
  <xsl:apply-templates select="Grouping|Token" mode="ProductionFormals"/>
  <CL value=");" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Grouping" mode="ProductionFormals">
  <NL hexvalue="20" />
  <COMMA value="," />
  <GroupingProduction value="ptree /*{@type}*/ x{position()}" />
</xsl:template>

<xsl:template match="Token" mode="ProductionFormals">
  <NL hexvalue="20" />
  <COMMA value="," />
  <TokenProduction value="token /*{@name}*/ x{position()}" />
</xsl:template>

<xsl:template match="@*">
<xsl:copy>
  <xsl:apply-templates select="@*"/>
</xsl:copy>
</xsl:template>

<xsl:template match="node()">
</xsl:template>

</xsl:transform>
