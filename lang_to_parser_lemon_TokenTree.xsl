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
    <IncludeOpen>
      <xsl:attribute name="value"><![CDATA[
%include
{
]]>        
      </xsl:attribute>
    </IncludeOpen>


    <Include>
      <xsl:attribute name="value"><![CDATA[
#include <assert.h>
#include <stdio.h> // parsetrace uses FILE
#include <stdlib.h>

#if INTERFACE
// Things to inject into makeheaders exported interface
#ifndef YYMALLOCARGTYPE
#define YYMALLOCARGTYPE size_t
#endif
  
#include "../tools/token.h"
#include "../tools/ptree.h"
#endif

// #include "arith.lemon.h"

// Removes LemonAlloc/LemonFree
#define arith_Lemon_ENGINEALWAYSONSTACK 1


]]>
      </xsl:attribute>
    </Include>

    <NL hexvalue = "0a" />
    <Header value='#include "{$LangName}.ptree.h"' />
    <NL hexvalue = "0a" />

    <IncludeClose>
      <xsl:attribute name="value"><![CDATA[
}
]]>        
      </xsl:attribute>
    </IncludeClose>

    
    <CodeOpen value="&#xA;%code&#xA;{{&#xA;" />

<Code>
      <xsl:attribute name="value">
  void <xsl:value-of select="$LangName"/>_parser_initialize(<xsl:value-of select="$LangName"/>_parser_type*p, ptree_context <xsl:value-of select="$LangName"/>_ptree_context)
  {
    <xsl:value-of select="$LangName"/>_LemonInit(&amp;p->lemon, <xsl:value-of select="$LangName"/>_ptree_context);
  }
  
  void <xsl:value-of select="$LangName"/>_parser_finalize(<xsl:value-of select="$LangName"/>_parser_type*p)
  {
    <xsl:value-of select="$LangName"/>_LemonFinalize(&amp;p->lemon);
  }

  void <xsl:value-of select="$LangName"/>_parser_parse(<xsl:value-of select="$LangName"/>_parser_type*p, int id, token t)
  {
    assert(id > 0);
    <xsl:value-of select="$LangName"/>_Lemon(&amp;p->lemon, id, t, 0);
  }

  ptree <xsl:value-of select="$LangName"/>_parser_tree(<xsl:value-of select="$LangName"/>_parser_type*p)
  {
    ptree tmp;
    token tok = token_create_novalue("");
    <xsl:value-of select="$LangName"/>_Lemon(&amp;p->lemon, 0, tok, &amp;tmp);
    return tmp;
  }

      </xsl:attribute>
</Code>

    <CodeClose value="&#xA;}}&#xA;"/>

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
  <ResultType value = "{@grouping}" />
  <ResultName value="(R)" />
  <NL hexvalue="20" />
  <Arrow value="::=" />
  <xsl:apply-templates select="Grouping|Token" mode="ListProductionGroupings"/>
  <DotNL hexvalue="2e0a" />
</xsl:template>

<xsl:template match="ListProduction">
  <NL hexvalue="0a" />
  <Label value="// ListProduction {@label} -&gt; {$LangName}_grouping_{@grouping}" />
  <NL hexvalue="0a" />
  <xsl:call-template name="production-definition" />

  <LB hexvalue="7b0a" /> 
  <xsl:apply-templates select="Grouping|Token" mode="ListProductionDescribe"/>
  
  <ES value = "  enum {{" />
  <NL hexvalue="0a" />
  <T value="    argument_arity = {count(Grouping[@position])}," />
  <NL hexvalue="0a" />
  <T value="    token_arity = {count(Token[@position])}," />
  <NL hexvalue="0a" />
  <T value="    total_arity = {count(Grouping[@position]) + count(Token[@position])}," />
  <NL hexvalue="0a" />
  <EE value = "  }};" />
  <NL hexvalue="0a" />

  <ListTmp value = '  ptree tmp[total_arity];&#x0a;' />

  <xsl:apply-templates select="Grouping|Token" mode="ListProductionAssign"/>
  
  <ResExpr value = '  R = {$LangName}_ptree_expression{count(Grouping[@position]) + count(Token[@position])}(ctx,&#x0a;        {$LangName}_grouping_{@grouping}' />


<!-- this is way harder than it should be

  <xsl:call-template name="ListProductionSelectGrouping">
    <xsl:with-param name="iter" select="1" />
    <xsl:with-param name="maxiter" select="count(Grouping) + count(Token) + 1" />
  </xsl:call-template>
-->

  <xsl:apply-templates select="Grouping|Token" mode="ListProductionSelect"/>
  
  <EndResExpr value=");&#x0a;" />

  <RB hexvalue="7d0a" />
</xsl:template>

<xsl:template match="Grouping" mode="ListProductionGroupings">
  <NL hexvalue="20" />
  <GroupingProduction value="{@type}(x{position()})" />
</xsl:template>

<xsl:template match="Token" mode="ListProductionGroupings">
  <NL hexvalue="20" />
  <TokenProduction value="{@name}(x{position()})" />
</xsl:template>

<xsl:template match="Grouping" mode="ListProductionDescribe">
  <N value = "  // Grouping {@type}(x{position()}) is" />
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

<xsl:template match="Grouping" mode="ListProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  tmp[{@position}-1] = x{position()};" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  list_free(x{position()});" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template name="ListProductionSelectGrouping" >
  <xsl:param name="iter" />
  <xsl:param name="maxiter" />

  <NL hexvalue="0a" />
  <MyPos value="myiter={$iter} of {$maxiter}" />

  <xsl:if test="$maxiter > $iter" >
  <NL hexvalue="0a" />
  <MyPos value="mypos={position()}" />
  <NL hexvalue="0a" />
  <ClaimedPos value="groupclaimedpos={Grouping/@position}" />
  <NL hexvalue="0a" />
  <ClaimedPos value="tokenclaimedpos={Token/@position}" />
  <NL hexvalue="0a" />

  
  <xsl:choose>
    <xsl:when test="Grouping/@position">
      <P value="  group tmp[{@position}-1] = x{position()};" />
    </xsl:when>
    <xsl:when test="Token/@position">
      <P value="  token tmp[{@position}-1] = x{position()};" />
    </xsl:when>

    <xsl:otherwise>
      <P value="  list_free(x{position()});" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />

  <xsl:call-template name="ListProductionSelectGrouping">
    <xsl:with-param name="iter" select="1 + $iter" />
    <xsl:with-param name="maxiter" select="$maxiter" />
  </xsl:call-template>


</xsl:if>
</xsl:template>


<xsl:template match="Token" mode="ListProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  tmp[{@position}-1] = list_from_token(x{position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>


<xsl:template match="Grouping" mode="AssignProductionAssign">
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

  <xsl:apply-templates select="Grouping|Token" mode="AssignProductionAssign"/>
  <LR hexvalue="7d0a" />  
</xsl:template>

</xsl:transform>
