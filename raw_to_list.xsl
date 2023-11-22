<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings"
               xmlns:ext="http://exslt.org/common"
               xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               extension-element-prefixes="str ext"
>

<!-- Takes a root RawText and hacks with the parens, emitting text -->

<xsl:output method="text"/>

<xsl:template match="RawText">
    
<text>&lt;?xml version="1.0" encoding="UTF-8"?&gt;
&lt;ListText&gt;</text>

<xsl:value-of select="str:replace(str:replace(.,'(','&lt;List&gt; '),')',' &lt;/List>')"/> 
<text>&lt;/ListText&gt;</text>
</xsl:template>

</xsl:transform>
  
  
