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
    <Header value="// Parser bison {$LangName}" />
    <NL hexvalue = "0a0a" />

    <IncludeOpen value="%code top&#xA;{{" />

    <Include>
      <xsl:attribute name="value"><![CDATA[
#include <assert.h>
#include <stdio.h> // parsetrace uses FILE
#include <stdlib.h>
  
#include "../tools/token.h"
#include "../tools/ptree.h"

]]>

#include "<xsl:value-of select='$LangName'/>.ptree.h"
#include "<xsl:value-of select='$LangName'/>.lexer.h"
#include "<xsl:value-of select='$LangName'/>.declarations.h"

 __attribute__((unused)) static int <xsl:value-of select='$LangName'/>_LemonFallback(int);

static void <xsl:value-of select='$LangName'/>_LemonFinalize(void *p);
static void <xsl:value-of select='$LangName'/>_LemonInit(void *, ptree_context);
static void <xsl:value-of select='$LangName'/>_Lemon(void*, int, token, ptree*);

      </xsl:attribute>
    </Include>

<InterfaceType>
      <xsl:attribute name="value">

__attribute__((unused))
static ptree parser_<xsl:value-of select="$LangName"/>_ptree_from_token(ptree_context ctx, enum <xsl:value-of select="$LangName"/>_token id, token tok)
{
  return <xsl:value-of select="$LangName"/>_ptree_from_token(ctx, id, tok.value, tok.width);
}

      </xsl:attribute>
</InterfaceType>

    <IncludeClose value="&#xA;}} // %code top" />
    
    <CodeOpen value="&#xA;%code&#xA;{{&#xA;" />
    <Code>
      <xsl:attribute name="value">

void yyerror (ptree* external_context, ptree_context regex_ptree_context, char const *s) {        
    fprintf (stderr, "%s\n", s); 
}

#if 0
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
    ptree tmp = ptree_failure();
    token tok = token_create_novalue("");
    <xsl:value-of select="$LangName"/>_Lemon(&amp;p->lemon, 0, tok, &amp;tmp);
    return tmp;
  }
#endif

      </xsl:attribute>
</Code>

    <CodeClose value="&#xA;}}&#xA;"/>

    <CodeTryingForEnd>
      <xsl:attribute name="value">

<!-- epilogue is undocumented, should probably be writing it after another %% -->
%code epilogue {
#if __STDC_HOSTED__
#include &lt;stdio.h&gt;
int <xsl:value-of select='$LangName'/>_parser_bison_type_header(void)
{
  return printf(
  "#ifndef <xsl:value-of select='$UpcaseLangName'/>_PARSER_BISON_T_INCLUDED\n"
  "#define <xsl:value-of select='$UpcaseLangName'/>_PARSER_BISON_T_INCLUDED\n"
  "\n"
  "enum {\n"
  "  <xsl:value-of select='$LangName'/>_parser_bison_type_align = %lu,\n"
  "  <xsl:value-of select='$LangName'/>_parser_bison_type_size = %lu,\n"
  "};\n"
  "struct <xsl:value-of select='$LangName'/>_parser_bison_state {\n"
  "    char _Alignas(<xsl:value-of select='$LangName'/>_parser_bison_type_align) data[<xsl:value-of select='$LangName'/>_parser_bison_type_size];\n"
  "};\n"
  "\n"
  "#endif\n",
  _Alignof(struct yypstate),
  sizeof(struct yypstate)) > 0 ? 0 : 1;
}
#endif
}
</xsl:attribute>
</CodeTryingForEnd>

<Configuration>
<xsl:attribute name="value">

// This is derived from a "skeleton" file called yacc.c
// Editing that file looks fairly hairy given it's in m4                                            

// bison configuration
// no global state                                              
%define api.pure full

// want to push tokens into the parser
%define api.push-pull push

// explicit                     
%define lr.type lalr                        

// prefixes the token yysymbol_kind_t, doesn't change api
//%define api.symbol.prefix {badger_}

// prefixes the tokentype enum in the header                      
%define api.token.prefix {BISON_}

// Use the same integers in the two enums
// is miscompiling BYTE00 = 0
// I think this also ignores the numeric assignment to tokens
// but with care is probably better
// %define api.token.raw

// Try to have more internal asserts
%define parse.assert true

// try to get more coherent errors
%define parse.error detailed
%define parse.lac full

 
%define api.prefix {<xsl:value-of select='$LangName'/>_bison_}
// %define api.value.type {token}

// This has trouble with different directories
// %header "<xsl:value-of select='$LangName'/>_parser.bison.h"

%union {
  token token;
  ptree ptree;
}

%start program // explicit

// lemon stashes this in the struct and makes it available to productions
// %extra_context {ptree_context <xsl:value-of select='$LangName'/>_ptree_context}
// this is passed to parse
// %extra_argument {ptree* external_context}

// bison is going to want to pass them both into every production
%parse-param {ptree* external_context} {ptree_context <xsl:value-of select='$LangName'/>_ptree_context}

</xsl:attribute>        
</Configuration>

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

// Separator required by bison
%%

]]></xsl:attribute>
    </Program>

    <xsl:apply-templates select="Productions"/>
  </TokenTree>
</xsl:template>

<!-- Tokens all have the same type, declared here -->
<xsl:template match="Tokens">
  <Header value="// Tokens" />
  <NL hexvalue = "0a" />
  <TokenType value = "// %token_type {{ token }}"/>
  <NL hexvalue = "0a" />
  <TokenPrefix value = "// %token_prefix TOKEN_ID_" />
  <NL hexvalue = "0a0a" />
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
  <Prog value="%type &lt;ptree&gt; program" />

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
  <TokenDecl literal="%token &lt;token&gt; {@name} // {@regex}{@literal}?" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="Precedence" >
  <PrecToken value=" {@name}" />
</xsl:template>
  
<xsl:template match="Left">
  <L value = "%left" />
  <xsl:apply-templates select="Token" mode="Precedence" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Right">
  <R value = "%right" />
  <xsl:apply-templates select="Token" mode="Precedence" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Nonassoc">
  <N value = "%nonassoc" />
  <xsl:apply-templates select="Token" mode="Precedence" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Grouping">
  <GroupingDecl value = "%type &lt;ptree&gt; {@name}"/>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template name="production-definition" >
  <ResultType value = "{@grouping}" />
  <Arrow value=" : " />
  <xsl:apply-templates select="Grouping|Token" mode="ListProductionGroupings"/>
  <NL hexvalue="0a" />
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


  <ResTmp value = '  $$ = {$LangName}_ptree_expression_create_uninitialised({$LangName}_ptree_context, {$LangName}_grouping_{@grouping}, total_arity);' />
  <NL hexvalue="0a" />

  <xsl:apply-templates select="Grouping|Token" mode="ListProductionAssign"/>
  <OutExpr value = '  if (external_context) {{*external_context = $$;}}' />
  <NL hexvalue="0a" />

  <xsl:apply-templates select="Grouping|Token" mode="ListProductionSelect"/>
  <RB hexvalue="7d0a" />
</xsl:template>

<xsl:template match="Grouping" mode="ListProductionGroupings">
  <GroupingProduction value="{@type} /*(${position()})*/" />
</xsl:template>

<xsl:template match="Token" mode="ListProductionGroupings">
  <TokenProduction value="{@name} /*(${position()})*/" />
</xsl:template>

<xsl:template match="Grouping" mode="ListProductionDescribe">
  <N value = "  // Grouping {@type} ${position()} is" />
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
  <N value="  // Token {@name} ${position()} is" />
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
      <P value="  {$LangName}_ptree_expression_initialise_element($$, {@position}-1, ${position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)${position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="ListProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      
      <T value = "  ptree tmp{position()} = parser_{$LangName}_ptree_from_token({$LangName}_ptree_context, {$LangName}_token_{@name}, x{position()});" />
      <NL hexvalue="0a" />
      <P value="  {$LangName}_ptree_expression_initialise_element($$, {@position}-1, tmp{position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)${position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Grouping" mode="AssignProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  $$ = ${position()};" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)${position()};" />
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="AssignProductionAssign">
  <xsl:choose>
    <xsl:when test="@position">
      <P value="  $$ = parser_{$LangName}_ptree_from_token({$LangName}_ptree_context, {$LangName}_token_{@name}, x{position()});" />
    </xsl:when>
    <xsl:otherwise>
      <P value="  (void)${position()};" />
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
