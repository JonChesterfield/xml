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

<xsl:template match="Tokens">
  <Header>
    <Lang value="// -*-c-*-" />
    <NL hexvalue = "0a" />
    <Include value='#include "{$LangName}.lexer.h"' />
    <NL hexvalue="0a" />
    <Assert value="#include &lt;assert.h&gt;" />
  </Header>

  <Prefix>
  <xsl:attribute name="value">
lexer_token_t <xsl:value-of select='$LangName' />_lexer_re2c_iterator_step(lexer_t lex,
                                             lexer_iterator_t* iter)
{
  (void)lex;
  const char* YYCURSOR = iter->cursor;
  const char* YYLIMIT = iter->end;
  const char* YYMARKER; (void)YYMARKER;

  enum <xsl:value-of select='$LangName' />_token id = <xsl:value-of select='$LangName' />_token_UNKNOWN;

  // re2c begin
  /*!re2c
      re2c:define:YYCTYPE = char;
      re2c:yyfill:enable = 0;
      
      // not delighted with a nul ending the string
      // I think re2c:eof = -1, deleting the $ rule and adding a bunch
      // of noise like re2c:define:YYPEEK allows dropping that constraint,
      // probably with a performance cost
      
      re2c:eof = 0; 
      re2c:indent:string = "  ";

      // rec2:header is unused, don't need to generate a header

      re2c:label:prefix = "re2c"; // yy is overused in lexers
  // End of common prefix
  </xsl:attribute>
  </Prefix>

  <TokenNames>
    <NL hexvalue = "0a" />
    <xsl:apply-templates select="Token" mode="Names"/>
  </TokenNames>
  <TokenDispatch>
    <NL hexvalue = "0a" />
    <xsl:apply-templates select="Token" mode="Dispatch"/>
  </TokenDispatch>

<Suffix>
  <xsl:attribute name="value">
  // Start of common suffix
  . { id = <xsl:value-of select='$LangName' />_token_UNKNOWN; goto match; }
  *    { goto failure; }
  $    { goto failure; }
  */
  // re2c finish

  // If we got here with none matching id is still unknown but YYCURSOR has
  // probably been mutated. Set up the variables to indicate failure.
failure:
  assert(id == <xsl:value-of select='$LangName' />_token_UNKNOWN);
  YYCURSOR = iter->cursor;

match:;
  const char * initial_cursor = iter->cursor;
  size_t width = YYCURSOR - initial_cursor;
  *iter = (lexer_iterator_t){.cursor = initial_cursor + width, .end = iter->end};
  return (lexer_token_t){
      .id = (size_t)id,
      .value = initial_cursor,
      .width = width,
  };
}
</xsl:attribute>
</Suffix>

</xsl:template>

<!-- re2c has different quoting rules to other regex engines
     Getting this to work consistently has been a pain as re2c fails to
     recognise things like \( or \xAB outside of its literal syntax.
     Also it hasn't implemented \e (can encode as a hex literal 5c1b for \ then 1b)

     Doesn't need C string literal escaping
     There's reasonable chance 'literal' with single quotes is right
     Need to escape ' values within the literal. Can optionally escape
     other values. Probably need to special case the */ sequence to avoid
     early-exit the comment from C's perspective.
     Using <LiteralRegex value="'{@literal}';" /> is working for some cases,
     but things like \t, which should be turning into matching a slash then a t,
     are probably being interpreted as matching a tab. In particular, '\e' warns
     about a useless escape, which suggests it thinks \ in literals are escapes
     Currently turning literal and hexliteral into regex then letting re2c
     turn that back into a state machine as that seems to have fewer edge cases.
-->
<xsl:template match="Token" mode="Names">  
  <ID value="  RE2C_{@name} = " />
  <xsl:choose>
    <xsl:when test="@regex" >
      <Regex value="{@regex};" />
    </xsl:when>

    <!--
        Tricky. Can't quote meta characters and call it a day because
        re2c doesn't accept \\n as a regex for \ followed by n
        It does recognise various escape sequences within a string literal,
        so also can't use '\n' for it. '\\n' would work.
        That's going to choke on single quotes in the literal, could replace those
        with a hex escape.
    -->
    <xsl:when test="@literal" >
      <!-- <LiteralRegex value="'{@literal}';" /> -->

        <LiteralRegex>
          <xsl:attribute name="value">
            <xsl:text>'</xsl:text>
            <xsl:call-template name="quotemeta">
              <xsl:with-param name="str" select="@literal"/>
            </xsl:call-template>
            <xsl:text>';</xsl:text>
          </xsl:attribute>
        </LiteralRegex>
    </xsl:when>


    <!-- re2c is rejecting \xAB syntax outside of literals, so wrap it in a ' -->
    <xsl:when test="@hexliteral" >
      <Quote value="'" />
      <HexLiteralRegex>
        <xsl:attribute name="value">
          <xsl:call-template name="hex_to_c_literal">
            <xsl:with-param name="str" select="@hexliteral"/>
          </xsl:call-template>
        </xsl:attribute>
      </HexLiteralRegex>
      <Quote value="'" />
      <Semicolon value=";" />
    </xsl:when>
    <xsl:otherwise>
      <Error value="$^" /> 
    </xsl:otherwise>
  </xsl:choose>
  <NL hexvalue = "0a" />
</xsl:template>

<xsl:template match="Token" mode="Dispatch">
  <ID value="  RE2C_{@name} {{ id = {$LangName}_token_{@name}; goto match; }}" />
  <NL hexvalue = "0a" />
</xsl:template>


<xsl:template match="@*">
<xsl:copy>
  <xsl:apply-templates select="@*"/>
</xsl:copy>
</xsl:template>

<xsl:template match="node()">
</xsl:template>

</xsl:transform>
