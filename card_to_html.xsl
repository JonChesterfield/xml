<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               >

  <xsl:output method="xml" omit-xml-declaration="yes" indent="yes"/>

  <xsl:template match="/Project" >
    <html>
      <head>        
        <!-- <meta http-equiv="refresh" content="5" /> -->
      </head>
      
      <style><![CDATA[
      html,
      body {
      width: 100%;
      height: 100%;
      }

   html {
     font-size: 40px;
   }

   h1 {
    font-size: 2.5rem;
   }

   body {
      //background-color: #8e44ad;
      //margin: 0;
      //display: flex;
      //justify-content: center;
      //align-items: center;
      }

      .Project {
      background-color: #fff;
      min-width: 100%;
      min-height: 400px;
      display: flex;
      flex-wrap: nowrap;
      overflow-x: auto;

      // trial and error involved in picking these, scroll-padding seems to disable the snap
      scroll-behavior: smooth;
      scroll-snap-type: x mandatory;
      // scroll-padding: 50%;
      }

      .Project::-webkit-scrollbar {
      display: none;
      }

      .card--content {
      background-color: #e74c3c;
      min-width: 200px;
      margin: 5px;
      height: 100vh;
      width: 100vw;
      scroll-snap-align: start;
      flex: 0 0 auto;
      }
      ]]>
      </style>
      <body>
        <section class="Project">
          <xsl:apply-templates select="node()|@*"/>
        </section>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="card" >
    <div class="card--content">
      <h1>
        <xsl:value-of select="@name" />
      </h1>
      <xsl:apply-templates select="node()"/>
    </div>      
  </xsl:template>

  <xsl:template match="task" >
    <div>
      <details>
      <summary>
        <xsl:value-of select="@name" />
      </summary>
      <xsl:apply-templates select="node()"/>
      </details>
    </div>
  </xsl:template>

  
<xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
</xsl:template>

  <xsl:template match="comment() | text() | processing-instruction()">
    <xsl:copy/>
  </xsl:template>


</xsl:transform>
