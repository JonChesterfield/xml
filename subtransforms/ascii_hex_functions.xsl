<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:str="http://exslt.org/strings" 
               xmlns:ext="http://exslt.org/common" 
               version="1.0"
               extension-element-prefixes="str ext">

  <xsl:variable name="hex">0123456789abcdef</xsl:variable>
  <!-- This might be turning newline into space -->
   <xsl:variable name="ascii"> !"#$%&amp;'()*+,-./0123456789:;&lt;=&gt;?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~</xsl:variable>

   <xsl:variable name="alpha">ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789</xsl:variable>
   
   <!-- https://stackoverflow.com/questions/70987650/convert-hex-to-ascii-characters-xslt -->
   <xsl:template name="hex-to-ascii">
      <xsl:param name="str"/>
      <xsl:if test="$str">
         <!-- extract first 2 digits -->
         <xsl:variable name="char1" select="substring($str, 1, 1)"/>
         <xsl:variable name="char2" select="substring($str, 2, 1)"/>
         <xsl:variable name="char12" select="substring($str, 3, 1)"/>

         <xsl:choose>
           <xsl:when test="$char12 = 'x9'" >
             <xsl:text>&#x9;</xsl:text>
           </xsl:when>
           <xsl:when test="$char12 = 'xa'" >
             <xsl:text>&#xa;</xsl:text>
           </xsl:when>
           <xsl:when test="$char12 = 'xd'" >
             <xsl:text>&#xd;</xsl:text>
           </xsl:when>
           <xsl:otherwise>
             <!-- get their hex values -->
             <xsl:variable name="val1" select="string-length(substring-before($hex, $char1))"/>
             <xsl:variable name="val2" select="string-length(substring-before($hex, $char2))"/>
             <!-- convert to dec value -->
             <xsl:variable name="dec-value" select="$val1 * 16 + $val2"/>
             <!-- get the corresponding ascii character -->
             <xsl:value-of select="substring($ascii, $dec-value - 31, 1)"/>
           </xsl:otherwise>
         </xsl:choose>
         <!-- recursive call with the rest of the hex string -->
         <xsl:call-template name="hex-to-ascii">
            <xsl:with-param name="str" select="substring($str, 3)"/>
         </xsl:call-template>
      </xsl:if>
   </xsl:template>

   <xsl:template name="ascii-to-hex">
      <xsl:param name="str"/>
      <xsl:if test="$str">
         <!-- extract first digit -->
         <xsl:variable name="char" select="substring($str, 1, 1)"/>

         <xsl:choose>
           <!-- Not delighted by this, there seems to be a few characters
                outside of the primary range that need to be remapped -->
           <xsl:when test="$char = '&#x9;'">
             <xsl:text>09</xsl:text>
           </xsl:when>
           <xsl:when test="$char = '&#xa;'">
             <xsl:text>0a</xsl:text>
           </xsl:when>
           <xsl:when test="$char = '&#xd;'">
             <xsl:text>0d</xsl:text>
           </xsl:when>
         <xsl:otherwise>
           <!-- find it in the ascii string -->
           <xsl:variable name="val" select="string-length(substring-before($ascii, $char))"/>
           <xsl:variable name="dec-value" select="$val + 32"/>
           <xsl:variable name="lo" select="$dec-value mod 16"/>
           <xsl:variable name="hi" select="floor($dec-value div 16)"/>
           <xsl:value-of select="substring($hex, $hi+1, 1)"/>
           <xsl:value-of select="substring($hex, $lo+1, 1)"/>
         </xsl:otherwise>
         </xsl:choose>
         <xsl:call-template name="ascii-to-hex">
            <xsl:with-param name="str" select="substring($str, 2)"/>
         </xsl:call-template>
      </xsl:if>
   </xsl:template>

   <!-- Considering a mixed ascii / hex encoding where alphanumeric are passed through
        unchanged, other characters are lowercase hex, and _ delimits the change -->

   <xsl:template name="ascii-to-mixed">
     <xsl:param name="str"/>
     <xsl:variable name="nested" >
       <xsl:call-template name="ascii-to-mixed-impl">
         <xsl:with-param name="str" select="$str"/>
       </xsl:call-template>
     </xsl:variable>
     <xsl:value-of select="str:replace($nested, '__', '')" />
   </xsl:template>

   <xsl:template name="ascii-to-mixed-impl">
      <xsl:param name="str"/>
      <xsl:if test="$str">
         <!-- extract first digit -->
         <xsl:variable name="char" select="substring($str, 1, 1)"/>
         <xsl:variable name="val" select="string-length(substring-before($ascii, $char))"/>
         <xsl:variable name="dec-value" select="$val + 32"/>

         <xsl:variable name="numeric" select="$dec-value >= 48 and $dec-value &lt;= 57" />
         <xsl:variable name="lowercase" select="$dec-value >= 97 and $dec-value &lt;= 122" />
         <xsl:variable name="uppercase" select="$dec-value >= 65 and $dec-value &lt;= 90" />
         <xsl:variable name="alphanumeric" select="$numeric or $lowercase or $uppercase" />

         <xsl:choose>
           <xsl:when test="$alphanumeric"><xsl:value-of select="$char"/></xsl:when>
           <xsl:otherwise>
             <xsl:text>_</xsl:text>
             <xsl:call-template name="ascii-to-hex">
               <xsl:with-param name="str" select="substring($str, 1, 1)"/>
             </xsl:call-template>
             <xsl:text>_</xsl:text>
           </xsl:otherwise>
         </xsl:choose>

         <xsl:call-template name="ascii-to-mixed-impl">
            <xsl:with-param name="str" select="substring($str, 2)"/>
         </xsl:call-template>
      </xsl:if>
   </xsl:template>

   <xsl:template name="mixed-to-ascii">
     <xsl:param name="str"/>
      <xsl:if test="$str">
         <xsl:variable name="char" select="substring($str, 1, 1)"/>
         <xsl:choose>
           <!-- If string is a single _, discard it -->
           <xsl:when test="$str = '_'"></xsl:when>
           <!-- if it starts with a _, optimistically assume there are two hex digits after -->
           <xsl:when test="$char = '_'">               
             <xsl:call-template name="hex-to-ascii">
               <xsl:with-param name="str" select="substring($str, 2, 2)"/>
             </xsl:call-template>
             <xsl:variable name="remainder" select="concat('_',substring($str, 4))"/>
             <xsl:call-template name="mixed-to-ascii">
               <xsl:with-param name="str" select="$remainder"/>
             </xsl:call-template>
           </xsl:when>
           <!-- Not a _, recurse on tail. -->
           <xsl:otherwise>
             <xsl:value-of select="$char"/>
             <xsl:call-template name="mixed-to-ascii">
               <xsl:with-param name="str" select="substring($str, 2)"/>
             </xsl:call-template>
           </xsl:otherwise>
         </xsl:choose>
      </xsl:if>
   </xsl:template>


   <xsl:template name="quotemeta">
     <!-- Named for perl. Escape ascii other than /[A-Za-z_0-9]/ -->
     <!-- xml/xslt probably mishandles nul, if it could be represented escape as \\x00
          not currently having a good time with non-ascii, but regex engines that can
          deal with that probably want the upper 128 values unchanged -->
     <xsl:param name="str"/>
     <xsl:if test="$str">
       <xsl:variable name="char" select="substring($str, 1, 1)"/>

         <xsl:choose>
           <!-- If string is a single _, discard it -->
           <xsl:when test="contains($alpha,$char)">
             <xsl:value-of select="$char"/>
           </xsl:when>
           <xsl:otherwise>
             <xsl:value-of select="concat('\\',$char)"/>
           </xsl:otherwise>
         </xsl:choose>
         <xsl:call-template name="quotemeta">
            <xsl:with-param name="str" select="substring($str, 2)"/>
         </xsl:call-template>       
     </xsl:if>     
   </xsl:template>

</xsl:transform>
