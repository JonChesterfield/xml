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

<!-- Maybe move this to ascii -->
<xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'" />
<xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
<xsl:variable name="UpcaseLangName" >
  <xsl:value-of select="translate($LangName,$lowercase,$uppercase)" />
</xsl:variable>


<xsl:template match="Tokens">
  <Header>
    <Header0 value ="#ifndef {$UpcaseLangName}_LEXER_DECLARATIONS_H_INCLUDED" />
    <NL hexvalue="0a" />
    <Header1 value ="#define {$UpcaseLangName}_LEXER_DECLARATIONS_H_INCLUDED" />
    <NL hexvalue="0a0a" />

    <Include value='#include &lt;stdbool.h&gt;' />
    <NL hexvalue="0a" />
    <Include value='#include &lt;stdint.h&gt;' />
    <NL hexvalue="0a0a" />
  </Header>

  <Lexer>
    <Comment value="// Lexer" />
    <NL hexvalue="0a" />

    <Include value='#include "../tools/lexer.t"' />
    <NL hexvalue="0a" />

    <EnumDecl value="enum" />
    <SP value = " " />
    <LangName value="{$LangName}_token;" />
    <NL hexvalue="0a" />

    <Decl value="lexer_t {$LangName}_lexer_create(void);" />
    <NL hexvalue="0a" />
    <Decl value="void {$LangName}_lexer_destroy(lexer_t);" />
    <NL hexvalue="0a" />
    <Decl value="bool {$LangName}_lexer_valid(lexer_t);" />
    <NL hexvalue="0a" />
    <Decl value="bool {$LangName}_lexer_discard_token(enum {$LangName}_token);" />
    <NL hexvalue="0a" />
    <Decl value="lexer_token_t {$LangName}_lexer_iterator_step(lexer_t, lexer_iterator_t *);" />
    <ValidDecl><xsl:attribute name="value" >
static inline bool <xsl:value-of select='$LangName' />_lexer_identifier_valid_token(uint64_t);
</xsl:attribute></ValidDecl>
    <NL hexvalue="0a" />
  </Lexer>

  <TokenEnum>
    <Comment value="// Enumeration" />
    <NL hexvalue="0a" />
    <EnumDecl value="enum" />
    <SP value = " " />
    <LangName value="{$LangName}_token" />

    <SP value = " " />
    <LB hexvalue = "7b0a" />
    <Default value="  {$LangName}_token_UNKNOWN = 0," />
    <NL hexvalue="0a" />
    <xsl:apply-templates select="Token" mode="Enumerate"/>
    <RB hexvalue = "7d" />
    <SC value=";" />

    <NL hexvalue="0a" />
    <Count value = "enum {{ {$LangName}_token_count = {count(Token) + 1} }};" />
    <NL hexvalue="0a" />
  </TokenEnum>
  <LexerInline>
  <ValidDefn><xsl:attribute name="value" >
static inline bool <xsl:value-of select='$LangName' />_lexer_identifier_valid_token(uint64_t id)
{
  return (id &gt; <xsl:value-of select='$LangName' />_token_UNKNOWN) &amp;&amp; (id &lt; <xsl:value-of select='$LangName' />_token_count);
}
</xsl:attribute></ValidDefn>
</LexerInline>

  <TokenNames>
    <NL hexvalue="0a" />
    <Comment value="// Names table" />
    <NL hexvalue="0a" />

    <Extern value="extern" />
    <SP value = " "/>
    <Type value="const char*" />
    <LangName value="{$LangName}_token_names[{$LangName}_token_count];" />
    <NL hexvalue="0a" />
  </TokenNames>

  <RegexTable >
    <NL hexvalue="0a" />
    <Comment value="// Regex table" />
    <NL hexvalue="0a" />

    <Extern value="extern" />
    <SP value = " "/>
    <Type value="const char*" />
    <LangName value="{$LangName}_regexes[{$LangName}_token_count];" />
    <NL hexvalue="0a" />
  </RegexTable>


  <NL hexvalue="0a0a" />
  <Footer>
    <Footer0 value="#endif /* {$UpcaseLangName}_LEXER_DECLARATIONS_H_INCLUDED */" />
  </Footer>
</xsl:template>

<xsl:template match="Token" mode="Enumerate">
  <Token>
    <SP value = "  "/>
    <LangName value="{$LangName}_token_{@name}" />
    <SP value = " " />
    <Assign value="="/>
    <SP value = " " />
    <Integer>
      <xsl:attribute name="value" >
        <xsl:value-of select="position()" />
      </xsl:attribute>
    </Integer>
    <comma value = "," />
    <NL hexvalue = "0a" />
  </Token>
</xsl:template>

<xsl:template match="@*">
<xsl:copy>
  <xsl:apply-templates select="@*"/>
</xsl:copy>
</xsl:template>

<xsl:template match="node()">
</xsl:template>

</xsl:transform>
