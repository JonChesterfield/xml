<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               extension-element-prefixes="str ext"
>

<!-- Takes a root RawText and hacks with the parens, emitting text -->

<xsl:output method="text"/>

<xsl:template match="/RawText">
    
<text>&lt;?xml version="1.0" encoding="UTF-8"?&gt;
&lt;ListText&gt;</text>
<!-- Might be a saner way to do this.--> 
<!--
<xsl:variable name="esc0" select="str:replace(., '_', '_5F')" />
<xsl:variable name="esc1" select="str:replace($esc0, '&lt;', '_3C')" />
<xsl:variable name="esc2" select="str:replace($esc1, '&gt;', '_3E')" />
<xsl:variable name="esc3" select="str:replace($esc2, '&amp;', '_26')" />

<xsl:variable name="esc4" select="str:replace($esc3, '-', '_2D')" />
<xsl:variable name="esc5" select="str:replace($esc4, ',', '_2C')" />
<xsl:variable name="esc6" select="str:replace($esc5, '+', '_2B')" />
<xsl:variable name="esc7" select="str:replace($esc6, '*', '_2A')" />

<xsl:variable name="esc8" select="str:replace($esc7, '?', '_3F')" />
<xsl:variable name="esc9" select="str:replace($esc8, '=', '_3D')" />
<xsl:variable name="escN" select="$esc9" />
-->
<xsl:variable name="lp" select="str:replace(., '(', '&lt;List&gt;' )" />
<xsl:variable name="rp" select="str:replace($lp, ')', '&lt;/List&gt;' )" />

<xsl:value-of select="$rp" />

<text>&lt;/ListText&gt;</text>
</xsl:template>

</xsl:transform>
