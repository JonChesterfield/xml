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

<xsl:include href="../subtransforms/ascii_hex_functions.xsl" />

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
    <Include value='#include "{$LangName}.lexer.h"' />
    <NL hexvalue="0a" />
    <NL hexvalue="0a" />
    <Include value='#include "../tools/lexer.posix.h"' />
    <NL hexvalue="0a" />
    <Include value='#include "../tools/lexer.re2.h"' />
    <NL hexvalue="0a" />
    <Include value='#include "../tools/lexer.re2c.h"' />
    <NL hexvalue="0a" />
    <Include value='#include "../tools/lexer.interp.h"' />
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
    <Default value='  [{$LangName}_token_UNKNOWN] = "UNKNOWN",' />
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
    <Default value='  [{$LangName}_token_UNKNOWN] = ".|[\n]",' />
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
    <Default value='  [{$LangName}_token_UNKNOWN] = ".|[\n]",' />
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
    <Default value='  [{$LangName}_token_UNKNOWN] = 0,' />
    <NL hexvalue="0a" />    
    <xsl:apply-templates select="Token" mode="Literals"/>
    <RB hexvalue = "7d" />
    <SC value=";" />
    <NL hexvalue="0a" />
  </LiteralsTable>

  <HexLiteralsTable >
    <NL hexvalue="0a" />
    <Comment value="// Language hex literals table" />
    <NL hexvalue="0a" />

    <Scope value="static __attribute__((unused))"/>
    <Type value="const char*" />
    <LangName value="{$LangName}_language_hex_literals[{$LangName}_token_count]" />
    <SP value = " " />
    <Assign value="="/>
    <SP value = " " />
    <LB hexvalue = "7b0a" />
    <Default value='  [{$LangName}_token_UNKNOWN] = 0,' />
    <NL hexvalue="0a" />    
    <xsl:apply-templates select="Token" mode="HexLiterals"/>
    <RB hexvalue = "7d" />
    <SC value=";" />
    <NL hexvalue="0a" />
  </HexLiteralsTable>


  <NL hexvalue="0a" />
  <Comment value="// Lexer instantiations" />
  <xsl:call-template name="LexerInstantiate">
    <xsl:with-param name="variant">posix</xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="LexerInstantiate">
    <xsl:with-param name="variant">re2</xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="LexerInstantiate">
    <xsl:with-param name="variant">re2c</xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="LexerInstantiate">
    <xsl:with-param name="variant">interp</xsl:with-param>
  </xsl:call-template>

  <Def value="#define LEXER_LANGUAGE {$LangName}" />
  <NL hexvalue="0a" />
  <Include value='#include "../tools/lexer.multi.h"' />

  <Lexer>
    <NL hexvalue="0a" />
    <Comment value="// Lexer using engine {$RegexEngine}" />
    <NL hexvalue="0a" />

    <xsl:variable name="upcase-engine" >
      <xsl:value-of select="translate($RegexEngine,$lowercase,$uppercase)" />
    </xsl:variable>

    <Pre value="#if !LEXER_{$upcase-engine}_ENABLE" />
    <NL hexvalue="0a" />
    <Err value='#error "Requested engine {$RegexEngine} is not enabled"' />
    <NL hexvalue="0a" />
    <Post value="#endif" />
    <NL hexvalue="0a" />

    <Defn value="lexer_t {$LangName}_lexer_create(void) {{ return {$LangName}_lexer_{$RegexEngine}_create(); }}" />
    <NL hexvalue="0a" />
    
    <Defn value="void {$LangName}_lexer_destroy(lexer_t lex) {{ {$LangName}_lexer_{$RegexEngine}_destroy(lex); }}" />
    <NL hexvalue="0a" />
    
    <Defn value="bool {$LangName}_lexer_valid(lexer_t lex) {{ return {$LangName}_lexer_{$RegexEngine}_valid(lex); }}" />
    <NL hexvalue="0a" />
    
    <Defn value="lexer_token_t {$LangName}_lexer_iterator_step(lexer_t lex, lexer_iterator_t *iter) {{ return {$LangName}_lexer_{$RegexEngine}_iterator_step(lex, iter); }}" />
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
    <!-- C thinks \ is a special thing and xsl does not
         It is thus tempting to replace \ with \\
         however this falls over badly in character classes
         since [\t] is a literal tab, and rewriting it to [\\t] is no good
         Going to use hex escapes on the C literal as that works everywhere,
         and the context-sensitivity of \ meaning different things inside/outside
         of a charset is difficult to model here.
    -->
      <xsl:when test="@regex" >
        <Regex>
          <xsl:attribute name="value">
            <xsl:text>"</xsl:text>
            <xsl:call-template name="ascii-to-escaped-c-string">
              <xsl:with-param name="str" select="@regex"/>
            </xsl:call-template>
            <xsl:text>",</xsl:text>
          </xsl:attribute>
        </Regex>
      </xsl:when>
      <xsl:otherwise>
        <Other value="0," />
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
      <xsl:when test="@literal" >
        <LiteralRegex>
          <xsl:attribute name="value">
            <xsl:text>"</xsl:text>
            <xsl:call-template name="doublequotemeta">
              <xsl:with-param name="str" select="@literal"/>
            </xsl:call-template>
            <xsl:text>",</xsl:text>
          </xsl:attribute>
        </LiteralRegex>
      </xsl:when>
      <xsl:otherwise>
        <Other value="0," />
      </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue = "0a" />
</xsl:template>

<xsl:template match="Token" mode="HexLiterals">
  <SP value="  " />
  <ID value="[{$LangName}_token_{@name}]" />
  <SP value = " " />
  <Assign value="=" />
  <SP value = " " />
  <xsl:choose>
      <xsl:when test="@hexliteral" >
        <HexLiteral>
          <xsl:attribute name="value">
            <xsl:text>"</xsl:text>
            <xsl:call-template name="hex_to_escaped_slash_c_literal">
              <xsl:with-param name="str" select="@hexliteral"/>
            </xsl:call-template>
            <xsl:text>",</xsl:text>
          </xsl:attribute>            
        </HexLiteral>
      </xsl:when>
      <xsl:otherwise>
        <Other value="0," />
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
        <Regex>
          <xsl:attribute name="value">
            <xsl:text>"</xsl:text>
            <xsl:call-template name="ascii-to-escaped-c-string">
              <xsl:with-param name="str" select="@regex"/>
            </xsl:call-template>
            <xsl:text>",</xsl:text>
          </xsl:attribute>
        </Regex>
      </xsl:when>

      <xsl:when test="@literal" >
        <LiteralRegex>
          <xsl:attribute name="value">
            <xsl:text>"</xsl:text>
            <xsl:call-template name="doublequotemeta">
              <xsl:with-param name="str" select="@literal"/>
            </xsl:call-template>
            <xsl:text>",</xsl:text>
          </xsl:attribute>
        </LiteralRegex>
      </xsl:when>

      <xsl:when test="@hexliteral" >
        <HexLiteral>
          <xsl:attribute name="value">
            <xsl:text>"</xsl:text>
            <xsl:call-template name="hex_to_escaped_slash_c_literal">
              <xsl:with-param name="str" select="@hexliteral"/>
            </xsl:call-template>
            <xsl:text>",</xsl:text>
          </xsl:attribute>            
        </HexLiteral>
      </xsl:when>

      <xsl:otherwise>
        <!-- Idea is to match nothing, should check whether that's the behaviour -->
        <Error value="$^" /> 
      </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue = "0a" />  
</xsl:template>

<xsl:template match="Token" mode="Discard">
  <ID value="    case {$LangName}_token_{@name}:" />
  <NL hexvalue="0a" />
</xsl:template>

<xsl:template match="TokenDiscard">
    <Comment value="// TokenDiscard start" />
    <NL hexvalue="0a" />
    <Decl value="bool {$LangName}_lexer_discard_token(enum {$LangName}_token id)" />
    <NL hexvalue="0a" />
    <OB value="{{" />
    <NL hexvalue="0a" />
    <OB value="  switch(id)" />
    <OB value="  {{" />
    <NL hexvalue="0a" />
    <xsl:apply-templates select="Token" mode="Discard"/>
    <CB value="    return true;&#xA;default:&#xA; return false;&#xA;" />
    <CB value="  }}" />
    <NL hexvalue="0a" />
    <CB value="}}" />
    <NL hexvalue="0a" />
    <Comment value="// TokenDiscard end" />
    <NL hexvalue="0a" />
</xsl:template>

<xsl:template name="LexerInstantiate" >
  <xsl:param name="variant"/>

  <xsl:variable name="upcase-variant" >
    <xsl:value-of select="translate($variant,$lowercase,$uppercase)" />
  </xsl:variable>

<LexerInstantiation>
  <NL hexvalue = "0a" />  
  <Pre value="#if LEXER_{$upcase-variant}_ENABLE" />

<Majority>
  <xsl:attribute name="value">
lexer_t <xsl:value-of select='$LangName' />_lexer_<xsl:value-of select='$variant' />_create(void)
{
  return lexer_<xsl:value-of select='$variant' />_create(<xsl:value-of select='$LangName' />_token_count, <xsl:value-of select='$LangName' />_regexes); 
}

void <xsl:value-of select='$LangName' />_lexer_<xsl:value-of select='$variant' />_destroy(lexer_t lex)
{
  lexer_<xsl:value-of select='$variant' />_destroy(lex);
}

bool <xsl:value-of select='$LangName' />_lexer_<xsl:value-of select='$variant' />_valid(lexer_t lex)
{
  return lexer_<xsl:value-of select='$variant' />_valid(lex); 
}
  </xsl:attribute>
</Majority>

  <Iterator>
    <xsl:choose>
      <xsl:when test="$variant = 're2c'" >
        <xsl:attribute name="value">
#include "<xsl:value-of select='$LangName' />.lexer_re2c_iterator.data"
        </xsl:attribute>        
      </xsl:when>
      <xsl:otherwise>
        <xsl:attribute name="value">
lexer_token_t <xsl:value-of select='$LangName' />_lexer_<xsl:value-of select='$variant' />_iterator_step(lexer_t lex, lexer_iterator_t *iter)
{
  return lexer_<xsl:value-of select='$variant' />_iterator_step(lex, iter);
  }
        </xsl:attribute>
      </xsl:otherwise>
    </xsl:choose>
  </Iterator>
  <NL hexvalue = "0a" />  
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
