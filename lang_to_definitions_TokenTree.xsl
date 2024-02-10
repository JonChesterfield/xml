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

<xsl:include href="./subtransforms/ascii_hex_functions.xsl" />

<xsl:variable name="LangName">
  <xsl:value-of select="/Language/LanguageName"></xsl:value-of>
</xsl:variable>

<xsl:variable name="RegexEngine">
  <xsl:value-of select="/Language/Lexer/Engine/@value"></xsl:value-of>
</xsl:variable>

<!-- Maybe move this to ascii -->
<xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'" />
<xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
<xsl:variable name="UpcaseLangName" >
  <xsl:value-of select="translate($LangName,$lowercase,$uppercase)" />
</xsl:variable>


<xsl:template match="Tokens">
  <Header>
    <Include value='#include "{$LangName}.declarations.h"' />
    <NL hexvalue="0a" />
    <NL hexvalue="0a" />
    <Include value='#include "../tools/lexer.posix.h"' />
    <NL hexvalue="0a" />
    <Include value='#include "../tools/lexer.re2.h"' />
    <NL hexvalue="0a" />
    <Include value='#include "../tools/lexer.re2c.h"' />
    <NL hexvalue="0a" />
    <Include value='#include "../tools/lexer.multi.h"' />
    <NL hexvalue="0a" />
  </Header>
  
  <TokenNames>
    <NL hexvalue="0a" />
    <Comment value="// Names table" />
    <NL hexvalue="0a" />

    <Type value="const char*" />
    <LangName value="{$LangName}_token_names[{$LangName}_token_count]" />
    <SP value = " " />
    <Assign value="="/>
    <SP value = " " />
    <LB hexvalue = "7b0a" />
    <Default value='  [arith_token_UNKNOWN] = "UNKNOWN",' />
    <NL hexvalue="0a" />
    <xsl:apply-templates select="Token" mode="Names"/>
    <RB hexvalue = "7d" />
    <SC value=";" />
  </TokenNames>

  <CombinedTable >
    <NL hexvalue="0a" />
    <Comment value="// Regex table" />
    <NL hexvalue="0a" />

    <Type value="const char*" />
    <LangName value="{$LangName}_regexes[{$LangName}_token_count]" />
    <SP value = " " />
    <Assign value="="/>
    <SP value = " " />
    <LB hexvalue = "7b0a" />
    <Default value='  [arith_token_UNKNOWN] = ".",' />
    <NL hexvalue="0a" />    
    <xsl:apply-templates select="Token" mode="Combined"/>
    <RB hexvalue = "7d" />
    <SC value=";" />
  </CombinedTable>
  
  <RegexTable >
    <NL hexvalue="0a" />
    <Comment value="// Language regex table" />
    <NL hexvalue="0a" />

    <Scope value="static __attribute__((unused))"/>
    <Type value="const char*" />
    <LangName value="{$LangName}_language_regexes[{$LangName}_token_count]" />
    <SP value = " " />
    <Assign value="="/>
    <SP value = " " />
    <LB hexvalue = "7b0a" />
    <Default value='  [arith_token_UNKNOWN] = ".",' />
    <NL hexvalue="0a" />
    <xsl:apply-templates select="Token" mode="Regexes"/>
    <RB hexvalue = "7d" />
    <SC value=";" />
  </RegexTable>

  <LiteralsTable >
    <NL hexvalue="0a" />
    <Comment value="// Language literals table" />
    <NL hexvalue="0a" />

    <Scope value="static __attribute__((unused))"/>
    <Type value="const char*" />
    <LangName value="{$LangName}_language_literals[{$LangName}_token_count]" />
    <SP value = " " />
    <Assign value="="/>
    <SP value = " " />
    <LB hexvalue = "7b0a" />
    <Default value='  [arith_token_UNKNOWN] = 0,' />
    <NL hexvalue="0a" />    
    <xsl:apply-templates select="Token" mode="Literals"/>
    <RB hexvalue = "7d" />
    <SC value=";" />
    <NL hexvalue="0a" />
  </LiteralsTable>

  <NL hexvalue="0a" />
  <Comment value="// Lexer instantiations" />
  <xsl:call-template name="LexerInstantiate">
    <xsl:with-param name="variant">posix</xsl:with-param>
    <xsl:with-param name="iterator">true</xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="LexerInstantiate">
    <xsl:with-param name="variant">re2</xsl:with-param>
    <xsl:with-param name="iterator">true</xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="LexerInstantiate">
    <xsl:with-param name="variant">re2c</xsl:with-param>
    <xsl:with-param name="iterator">false</xsl:with-param>
  </xsl:call-template>

  <Lexer>
    <NL hexvalue="0a" />
    <Comment value="// Lexer using engine {$RegexEngine}" />
    <NL hexvalue="0a" />

    <Defn value="lexer_t {$LangName}_lexer_create(void) {{ return lexer_{$RegexEngine}_create({$LangName}_token_count, {$LangName}_regexes); }}" />
    <NL hexvalue="0a" />
    
    <Defn value="void {$LangName}_lexer_destroy(lexer_t lex) {{ lexer_{$RegexEngine}_destroy(lex); }}" />
    <NL hexvalue="0a" />
    
    <Defn value="bool {$LangName}_lexer_valid(lexer_t lex) {{ return lexer_{$RegexEngine}_valid(lex); }}" />
    <NL hexvalue="0a" />
    
    <Defn value="lexer_token_t {$LangName}_lexer_iterator_step(lexer_t lex, lexer_iterator_t *iter) {{ return lexer_{$RegexEngine}_iterator_step(lex, iter); }}" />
    <NL hexvalue="0a" />

    <NL hexvalue="0a" />
  </Lexer>


  
  <NL hexvalue="0a0a" />
</xsl:template>

<xsl:template match="Token" mode="Names">
  <SP value="  " />
  <ID value="[{$LangName}_token_{@name}]" />
  <SP value = " " />
  <Assign value="=" />
  <SP value = " " />
  <N value='"{@name}"' />
  <comma value = "," />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="Token" mode="Regexes">
  <SP value="  " />
  <ID value="[{$LangName}_token_{@name}]" />
  <SP value = " " />
  <Assign value="=" />
  <SP value = " " />
  <xsl:choose>
      <xsl:when test="@regex" >
        <Regex value='"{@regex}",' />
      </xsl:when>
      <xsl:when test="@literal" >
        <Literal value="0," />
      </xsl:when>
      <xsl:otherwise>
        <Error value="error_no_regex_or_literal," />
      </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue = "0a" />
</xsl:template>

<xsl:template match="Token" mode="Literals">
  <SP value="  " />
  <ID value="[{$LangName}_token_{@name}]" />
  <SP value = " " />
  <Assign value="=" />
  <SP value = " " />
  <xsl:choose>
      <xsl:when test="@regex" >
        <Regex value='0,' />
      </xsl:when>
      <xsl:when test="@literal" >
        <Literal value='"{@literal}",' />
      </xsl:when>
      <xsl:otherwise>
        <Error value="error_no_regex_or_literal," />
      </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue = "0a" />
</xsl:template>

<xsl:template match="Token" mode="Combined">
  <SP value="  " />
  <ID value="[{$LangName}_token_{@name}]" />
  <SP value = " " />
  <Assign value="=" />
  <SP value = " " />
  <xsl:choose>
      <xsl:when test="@regex" >
        <Regex value='"{@regex}",' />
      </xsl:when>
      <xsl:when test="@literal" >
        <LiteralRegex>
          <xsl:attribute name="value">
            <xsl:text>"</xsl:text>
            <xsl:call-template name="quotemeta">
              <xsl:with-param name="str" select="@literal"/>
            </xsl:call-template>
            <xsl:text>",</xsl:text>
          </xsl:attribute>
        </LiteralRegex>
      </xsl:when>
      <xsl:otherwise>
        <!-- Idea is to match nothing, should check whether that's the behaviour -->
        <Error value="$^" /> 
      </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue = "0a" />  
</xsl:template>




<xsl:template name="LexerInstantiate" >
  <xsl:param name="variant"/>
  <xsl:param name="iterator"/>

  <xsl:variable name="upcase-variant" >
    <xsl:value-of select="translate($variant,$lowercase,$uppercase)" />
  </xsl:variable>

<LexerInstantiation>
  <NL hexvalue = "0a" />  
  <Pre value="#if LEXER_{$upcase-variant}_ENABLE" />
  <NL hexvalue = "0a" />  
<Majority>
  <xsl:attribute name="value">
lexer_t <xsl:value-of select='$LangName' />_lexer_<xsl:value-of select='$variant' />_create(void)
{
  return lexer_<xsl:value-of select='$variant' />_create(<xsl:value-of select='$LangName' />_token_count, <xsl:value-of select='$LangName' />_regexes); 
}

void <xsl:value-of select='$LangName' />_<xsl:value-of select='$variant' />_lexer_destroy(lexer_t lex)
{
  lexer_<xsl:value-of select='$variant' />_destroy(lex);
}

bool <xsl:value-of select='$LangName' />_<xsl:value-of select='$variant' />_lexer_valid(lexer_t lex)
{
  return lexer_<xsl:value-of select='$variant' />_valid(lex); 
}
  </xsl:attribute>
</Majority>

<xsl:if test="$iterator = 'true'" >
  <Iterator>
    <xsl:attribute name="value">
lexer_token_t <xsl:value-of select='$LangName' />_<xsl:value-of select='$variant' />_lexer_iterator_step(lexer_t lex, lexer_iterator_t *iter)
{
  return lexer_<xsl:value-of select='$variant' />_iterator_step(lex, iter);
  }
    </xsl:attribute>
  </Iterator>
</xsl:if>

<Post value="#endif" />
<NL hexvalue = "0a" />  
</LexerInstantiation>

</xsl:template>


<xsl:template match="@*">
<xsl:copy>
  <xsl:apply-templates select="@*"/>
</xsl:copy>
</xsl:template>

<xsl:template match="node()">
</xsl:template>

</xsl:transform>
