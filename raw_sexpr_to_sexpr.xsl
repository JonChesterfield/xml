<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
>

<!-- Takes a root RawText and hacks with the parens, emitting text -->

<xsl:output method="text"/>

<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="xsl:text"/>

<xsl:template match="/RawText">

<xsl:value-of select="." />

</xsl:template>


</xsl:transform>
