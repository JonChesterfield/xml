<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               >

  <xsl:output method="text"/>
  <xsl:strip-space elements="*"/>

  <xsl:template match="/TokenList">
       <xsl:apply-templates select="node()|@*"/>
  </xsl:template>

  <xsl:template match="node()">
    <xsl:choose>
      <xsl:when test="@value" >
        <xsl:value-of select="@value" />
      </xsl:when>
      <xsl:when test="@literal" >
        <xsl:value-of select="@literal" />
      </xsl:when>
    </xsl:choose>
  </xsl:template>
</xsl:transform>
