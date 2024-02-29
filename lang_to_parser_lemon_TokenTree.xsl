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

<xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'" />
<xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
<xsl:variable name="UpcaseLangName" >
  <xsl:value-of select="translate($LangName,$lowercase,$uppercase)" />
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

]]>

typedef union <xsl:value-of select="$LangName"/>_parser_u <xsl:value-of select="$LangName"/>_parser_type;

#endif // end INTERFACE

      </xsl:attribute>
    </Include>


    <NL hexvalue = "0a" />
    <OnStack value='#define {$LangName}_Lemon_ENGINEALWAYSONSTACK 1 // Remove LemonAlloc' />
    <NL hexvalue = "0a" />
    <Header value='#include "../tools/token.h"' />
    <NL hexvalue = "0a" />
    <Header value='#include "{$LangName}.ptree.h"' />
    <NL hexvalue = "0a" />
    <Header value='#include "{$LangName}.lexer.declarations.h"' />
    <NL hexvalue = "0a" />
    <Header value='#include "{$LangName}.declarations.h"' />
    <NL hexvalue = "0a" />
    <DisableFallback value="__attribute__((unused)) static int {$LangName}_LemonFallback(int);" />
    <NL hexvalue = "0a" />
    <StaticFinalize value="static void {$LangName}_LemonFinalize(void *p);" />
    <NL hexvalue = "0a" />
    <StaticInit value="static void {$LangName}_LemonInit(void *, ptree_context);" />
    <NL hexvalue = "0a" />
    <StaticParse value="static void {$LangName}_Lemon(void*, int, token, ptree*);" />

<InterfaceType>
      <xsl:attribute name="value">

__attribute__((unused))
static ptree parser_<xsl:value-of select="$LangName"/>_ptree_from_token(ptree_context ctx, enum <xsl:value-of select="$LangName"/>_token id, token tok)
{
  return <xsl:value-of select="$LangName"/>_ptree_from_token(ctx, id, tok.value, tok.width);
}

      </xsl:attribute>
</InterfaceType>

    <IncludeClose>
      <xsl:attribute name="value"><![CDATA[
}
]]>        
      </xsl:attribute>
    </IncludeClose>

    
    <CodeOpen value="&#xA;%code&#xA;{{&#xA;" />
    <Code>
      <xsl:attribute name="value">

#include "<xsl:value-of select='$LangName'/>_parser.lemon.h"

  struct <xsl:value-of select="$LangName"/>_parser_s {
    char _Alignas(_Alignof(struct yyParser)) data[sizeof(struct yyParser)];
  };

  union <xsl:value-of select="$LangName"/>_parser_u
  {
    struct yyParser lemon;
    struct <xsl:value-of select="$LangName"/>_parser_s state;
  };

  _Static_assert(sizeof(union <xsl:value-of select="$LangName"/>_parser_u) == sizeof(struct yyParser),"");

  _Static_assert(_Alignof(union <xsl:value-of select="$LangName"/>_parser_u) == _Alignof(struct yyParser),"");



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


#if __STDC_HOSTED__
#include &lt;stdio.h&gt;
int <xsl:value-of select='$LangName'/>_Lemon_parser_header(void)
{
  return printf(
  "#ifndef <xsl:value-of select='$UpcaseLangName'/>_PARSER_LEMON_H_INCLUDED\n"
  "#define <xsl:value-of select='$UpcaseLangName'/>_PARSER_LEMON_H_INCLUDED\n"
  "\n"
  "union <xsl:value-of select='$LangName'/>_parser_u;\n"
  "typedef union <xsl:value-of select='$LangName'/>_parser_u <xsl:value-of select='$LangName'/>_parser_type;\n"
  "\n"
  "typedef struct <xsl:value-of select='$LangName'/>_parser_s <xsl:value-of select='$LangName'/>_parser_s;\n"
  "enum {\n"
  "  <xsl:value-of select='$LangName'/>_parser_type_align = %lu,\n"
  "  <xsl:value-of select='$LangName'/>_parser_type_size = %lu,\n"
  "};\n"
  "struct <xsl:value-of select='$LangName'/>_parser_s {\n"
  "    char _Alignas(<xsl:value-of select='$LangName'/>_parser_type_align) data[<xsl:value-of select='$LangName'/>_parser_type_size];\n"
  "};\n"
  "\n"
  "#endif\n",
  _Alignof(struct yyParser),
  sizeof(struct yyParser)) > 0 ? 0 : 1;
}
#endif

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
    <ExtraArg value = "%extra_argument {{ptree* external_context}}" />
    <NL hexvalue = "0a" />
    <ExtraContext value = "%extra_context {{ptree_context {$LangName}_ptree_context}}" />
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
  <Prog value="%type program {{ ptree }}" />

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



  <ResTmp value = '  R = {$LangName}_ptree_expression_create_uninitialised({$LangName}_ptree_context, {$LangName}_grouping_{@grouping}, total_arity);' />
  <NL hexvalue="0a" />

  <xsl:apply-templates select="Grouping|Token" mode="ListProductionAssign"/>
  <OutExpr value = '  if (external_context) {{*external_context = R;}}' />
  <NL hexvalue="0a" />
  <!--
      <ResExpr value = '  return R;' />
      <NL hexvalue="0a" />
  -->

  <xsl:apply-templates select="Grouping|Token" mode="ListProductionSelect"/>
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
      <P value="  {$LangName}_ptree_expression_initialise_element(R, {@position}-1, x{position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="ListProductionAssign">
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


<xsl:template match="Grouping" mode="AssignProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  R = x{position()};" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="AssignProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  R = parser_{$LangName}_ptree_from_token({$LangName}_ptree_context, {$LangName}_token_{@name}, x{position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)x{position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="AssignProduction">
  <NL hexvalue="0a" />
  <Label value="// AssignProduction {@label} -&gt; {$LangName}_grouping_{@grouping}" />
  <NL hexvalue="0a" />
  <xsl:call-template name="production-definition" />
  <LB hexvalue="7b0a" />

  <xsl:apply-templates select="Grouping|Token" mode="AssignProductionAssign"/>
  <LR hexvalue="7d0a" />  
</xsl:template>

</xsl:transform>
