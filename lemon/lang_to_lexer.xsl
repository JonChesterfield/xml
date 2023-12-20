<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
               >
<xsl:output method="xml" indent="yes"/>

<xsl:template match="/Language">
  <Flex>
    <xsl:apply-templates select="node()|@*"/>
  </Flex>
</xsl:template>

<xsl:template match="Tokens">
<xsl:text><![CDATA[#include <cassert>
#include <cstdlib>

#include "token.h"

#include "lexer_instance.hpp"

#include <vector>
#include <iostream>

]]></xsl:text>

<xsl:text>// Enumeration&#xA;</xsl:text>
<xsl:text>enum {
  TOKEN_ID_UNKNOWN = 0,</xsl:text>
<xsl:apply-templates select="node()|@*" mode="Enumerate"/>
<xsl:text>};&#xA;</xsl:text>

<xsl:text>// Names table</xsl:text>
<xsl:text>&#xA;</xsl:text>
<xsl:text>const char* token_names[] = {</xsl:text>
<xsl:text>&#xA;</xsl:text>
<xsl:text>    [TOKEN_ID_UNKNOWN] = "TOKEN_ID_UNKNOWN",</xsl:text>
    <xsl:apply-templates select="node()|@*" mode="Names"/>
<xsl:text>};
enum { token_names_size = sizeof(token_names) / sizeof(token_names[0]) };
</xsl:text>

<xsl:text>// Regex table</xsl:text>
<xsl:text>&#xA;</xsl:text>
    
<xsl:text>const char* regexes[] = {</xsl:text>
<xsl:text>&#xA;</xsl:text>
<xsl:text>    [TOKEN_ID_UNKNOWN] = ".",</xsl:text>
    <xsl:apply-templates select="node()|@*" mode="Regexes"/>

<xsl:text>};
enum { regexes_size = sizeof(regexes) / sizeof(regexes[0]) };
</xsl:text>

<xsl:text>// Literals table</xsl:text>
<xsl:text>&#xA;</xsl:text>
    
<xsl:text>const char* literals[] = {</xsl:text>
<xsl:text>&#xA;</xsl:text>
<xsl:text>    [TOKEN_ID_UNKNOWN] = nullptr,</xsl:text>
    <xsl:apply-templates select="node()|@*" mode="Literals"/>

<xsl:text>};
enum { literals_size = sizeof(literals) / sizeof(literals[0]) };
</xsl:text>





<xsl:text><![CDATA[
static_assert((size_t)regexes_size == (size_t)literals_size, "");
static_assert((size_t)regexes_size == (size_t)token_names_size, "");

static bool is_regex(int index) { return regexes[index] != nullptr; }
static bool is_literal(int index) { return literals[index] != nullptr; }

int main() {

  for (int i = 0; i < token_names_size; i++)
  {
    assert(is_regex(i) || is_literal(i));
    assert(!(is_regex(i) && is_literal(i)));
  }

  std::vector<char> all_stdin;
  std::streamsize buffer_sz = 4 * 1024;
  std::vector<char> buffer(buffer_sz);
  all_stdin.reserve(buffer_sz);

  auto rdbuf = std::cin.rdbuf();
  while (auto cnt_char = rdbuf->sgetn(buffer.data(), buffer_sz))
  {
    all_stdin.insert(all_stdin.end(), buffer.data(), buffer.data() + cnt_char);
  }

  using LexerType = lexer_instance<regexes_size, token_names, regexes, literals>;
  auto lexer = LexerType(all_stdin.data(), all_stdin.size());
  if (!lexer) {
    return 1;
  }

  printf(R"(<?xml version="1.0" encoding="UTF-8"?>)" "\n");
  printf("<TokenList>\n");
  while (lexer) {
    token tok = lexer.next();
    if (token_empty(tok)) {
      return 2;
    }
    int index = tok.name;
    printf("  <%s %s = \"", token_names[index], 
           (is_regex(index) ? "value" : "literal"));
    const char *c = tok.value_start;
    while (c != tok.value_end) {
      printf("%c", *c++);
    }
    printf("\" />\n");
  }
  printf("</TokenList>\n");
  return 0;
}

]]></xsl:text>
</xsl:template>

<xsl:template match="Token" mode="Enumerate">
<xsl:text>TOKEN_ID_</xsl:text>
<xsl:value-of select="@name" />      
<xsl:text>,</xsl:text>
</xsl:template>

    
<xsl:template match="Token" mode="Regexes">
  <xsl:text>  [TOKEN_ID_</xsl:text><xsl:value-of select="@name" /><xsl:text>]</xsl:text>
  <xsl:text>=</xsl:text>

  <xsl:choose>
      <xsl:when test="@regex" >
        <xsl:text>R"(</xsl:text><xsl:value-of select="@regex" /><xsl:text>)",</xsl:text>          
      </xsl:when>
      <xsl:when test="@literal" >
        <xsl:text>nullptr,</xsl:text>
      </xsl:when>
      <xsl:otherwise>
          <xsl:text>error_no_regex_or_literal,</xsl:text>
      </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="Token" mode="Literals">
  <xsl:text>  [TOKEN_ID_</xsl:text><xsl:value-of select="@name" /><xsl:text>]</xsl:text>
  <xsl:text>=</xsl:text>

  <xsl:choose>
      <xsl:when test="@regex" >
        <xsl:text>nullptr,</xsl:text>
      </xsl:when>
      <xsl:when test="@literal" >
        <xsl:text>R"(</xsl:text><xsl:value-of select="@literal" /><xsl:text>)",</xsl:text>
      </xsl:when>
      <xsl:otherwise>
          <xsl:text>error_no_regex_or_literal,</xsl:text>
      </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="Token" mode="Names">
  <xsl:text>  [TOKEN_ID_</xsl:text><xsl:value-of select="@name" /><xsl:text>]</xsl:text>
  <xsl:text>=</xsl:text>
  <xsl:text>"</xsl:text><xsl:value-of select="@name" /><xsl:text>",</xsl:text>
</xsl:template>


<xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
</xsl:template>


</xsl:transform>
