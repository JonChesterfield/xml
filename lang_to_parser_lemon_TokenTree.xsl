<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >
<xsl:output method="xml" indent="yes"/>

<xsl:variable name="LangName">
  <xsl:value-of select="/Language/LanguageName"></xsl:value-of>
</xsl:variable>

<xsl:template match="/Language">
  <TokenTree>
    <Header value="// Parser lemon {$LangName}" />
    <NL hexvalue = "0a0a" />
    <Include>
      <xsl:attribute name="value"><![CDATA[
%include
{
#include <assert.h>
#include <stdio.h> // parsetrace uses FILE
#include <stdlib.h>

#if INTERFACE
// Things to inject into makeheaders exported interface
#ifndef YYMALLOCARGTYPE
#define YYMALLOCARGTYPE size_t
#endif
  
#include "../tools/token.h"
#include "../tools/list.h"
#endif

// #include "arith.lemon.h"

}]]>
      </xsl:attribute>
    </Include>
    <NL hexvalue = "0a" />
    <Header value="// lemon configuration" />
    <NL hexvalue = "0a" />
    <ParseName value="%name {$LangName}_Lemon" />
    <NL hexvalue = "0a" />
    <StartSymbol value = "%start_symbol program // explicit" />
    <NL hexvalue = "0a" />
    <ExtraArg value = "%extra_argument {{list* external_context}}" />
    <NL hexvalue = "0a" />
    <ErrorMessage value = '%syntax_error {{ fprintf(stderr, "Syntax error\n"); }}' />
    <NL hexvalue = "0a0a" />

    <xsl:apply-templates select="Tokens"/>
    <xsl:apply-templates select="Precedences"/>
    <xsl:apply-templates select="Groupings"/>

    <Program>
      <xsl:attribute name="value"><![CDATA[
// Current thinking is to provide a different interface implemented
// in terms of the lemon one, much like the lexer interface is
// implemented on top of some regex engine
// Maybe also therefore generate a header/source pair for data
// descriptions (the regex/literal strings, enums for terminals
// and non-terminals etc)
// Can give the terminals names as well, PLUS(D) etc

program ::= expr(A). {
   printf("Invoking program\n");
   *external_context = A;
   printf("\n");
}
]]></xsl:attribute>
    </Program>

    <xsl:apply-templates select="Productions"/>
  </TokenTree>
</xsl:template>

<!-- Tokens all have the same type, declared here -->
<xsl:template match="Tokens">
  <Header value="// Tokens" />
  <NL hexvalue = "0a" />
  <TokenType value = "%token_type {{ token }}"/>
  <NL hexvalue = "0a" />
  <TokenPrefix value = "%token_prefix TOKEN_ID_" />
  <NL hexvalue = "0a" />
  <NL hexvalue = "0a" />
  <xsl:apply-templates select="Token"/>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Precedences">
  <Header value="// Precedences" />
  <NL hexvalue = "0a" />
  <xsl:apply-templates select="Left|Right|Nonassoc"/>
  <NL hexvalue = "0a" />
</xsl:template>

<xsl:template match="Groupings">
  <Header value="// Groupings" />
  <NL hexvalue = "0a" />
  <xsl:apply-templates select="Grouping"/>
  <NL hexvalue="0a" />
</xsl:template>

<!--
    Lemon uses named arguments (A, B similar) and Bison uses $1, $2
    Unfortunately bison will accept $0, but it means result-of-previous,
    and negatives index into the stack. Not going to use that here.
    The result is $$ for bison, or whatever name it was given for lemon
    $<itype>1 style things let you specify the type but I think it means
    dubious punning things through a union if you get it wrong.
    Single result to each rule, might call it R or similar for lemon, language
    doesn't need to give it a name.
    
-->
     
<xsl:template match="Productions">
  <Header value="// Productions" />
  <NL hexvalue = "0a" />

  <xsl:apply-templates select="ListProduction|AssignProduction"/>
  
</xsl:template>


<xsl:template match="Token">
  <TokenDecl literal="%token {@name}. // {@regex}{@literal}" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="Precedence" >
  <PrecToken value=" {@name}" />
</xsl:template>
  
<xsl:template match="Left">
  <L value = "%left" />
  <xsl:apply-templates select="Token" mode="Precedence" />
  <DotNL hexvalue="2e0a" />
</xsl:template>

<xsl:template match="Right">
  <R value = "%right" />
  <xsl:apply-templates select="Token" mode="Precedence" />
  <DotNL hexvalue="2e0a" />
</xsl:template>

<xsl:template match="Nonassoc">
  <N value = "%nonassoc" />
  <xsl:apply-templates select="Token" mode="Precedence" />
  <DotNL hexvalue="2e0a" />
</xsl:template>

<xsl:template match="Grouping">
  <GroupingDecl value = "%type {@name} {{ {@type} }}"/>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template name="production-definition" >
  <ResultType value = "{@type}" />
  <ResultName value="(R)" />
  <NL hexvalue="20" />
  <Arrow value="::=" />
  <xsl:apply-templates select="Argument|Token" mode="ListProductionArguments"/>
  <DotNL hexvalue="2e0a" />
</xsl:template>

<xsl:template match="ListProduction">
  <NL hexvalue="0a" />
  <Label value="// ListProduction {@label}" />
  <NL hexvalue="0a" />
  <xsl:call-template name="production-definition" />

  <LB hexvalue="7b0a" /> 
  <xsl:apply-templates select="Argument|Token" mode="ListProductionDescribe"/>
  
  <ES value = "  enum {{" />
  <NL hexvalue="0a" />
  <T value="    argument_arity = {count(Argument[@position])}," />
  <NL hexvalue="0a" />
  <T value="    token_arity = {count(Token[@position])}," />
  <NL hexvalue="0a" />
  <T value="    total_arity = argument_arity + token_arity," />
  <NL hexvalue="0a" />
  <EE value = "  }};" />
  <NL hexvalue="0a" />

  <ListTmp value = '  list tmp = list_make_uninitialised("{@label}", total_arity);&#x0a;' />

  <xsl:apply-templates select="Argument|Token" mode="ListProductionAssign"/>
  
  <ReturnTmp value = '  R = tmp;&#x0a;' />

  <RB hexvalue="7d0a" />
</xsl:template>

<xsl:template match="Argument" mode="ListProductionArguments">
  <NL hexvalue="20" />
  <ArgumentProduction value="{@type}(x{position()})" />
</xsl:template>

<xsl:template match="Token" mode="ListProductionArguments">
  <NL hexvalue="20" />
  <TokenProduction value="{@name}(x{position()})" />
</xsl:template>

<xsl:template match="Argument" mode="ListProductionDescribe">
  <N value = "  // Argument {@type}(x{position()}) is" />
  <xsl:choose>
    <xsl:when test="@position">
      <P value=" used at position {@position}" />
    </xsl:when>
    <xsl:otherwise>
      <P value=" dead, needs deallocation" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="ListProductionDescribe">
  <N value="  // Token {@name}(x{position()}) is" />
  <xsl:choose>
    <xsl:when test="@position">
      <P value=" used at position {@position}" />
    </xsl:when>
    <xsl:otherwise>
      <P value=" dead, needs deallocation" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Argument" mode="ListProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  tmp.elts[{@position}-1] = x{position()};" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  list_free(x{position()});" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="ListProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  tmp.elts[{@position}-1] = list_from_token(x{position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>


<xsl:template match="Argument" mode="AssignProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  R = x{position()};" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  list_free(x{position()});" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="AssignProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  R = list_from_token(x{position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="AssignProduction">
  <NL hexvalue="0a" />
  <Label value="// AssignProduction {@label}" />
  <NL hexvalue="0a" />
  <xsl:call-template name="production-definition" />
  <LB hexvalue="7b0a" />

  <xsl:apply-templates select="Argument|Token" mode="AssignProductionAssign"/>
  <LR hexvalue="7d0a" />  
</xsl:template>

</xsl:transform>
