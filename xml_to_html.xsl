<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               >

  <xsl:output method="xml" omit-xml-declaration="yes" />

  <!-- Need to strip the md namespace somewhere, this isn't ideal but is better than nowhere -->
  <!-- Maybe in cmark -> md -->
  <xsl:template match="*">
    <xsl:element name="{local-name()}">
      <xsl:apply-templates select="@* | node()"/>
    </xsl:element>
  </xsl:template>

  <xsl:template match="@*">
    <xsl:attribute name="{local-name()}">
      <xsl:value-of select="."/>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="comment() | text() | processing-instruction()">
    <xsl:copy/>
  </xsl:template>

</xsl:transform>
