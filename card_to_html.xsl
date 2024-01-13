<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               >

  <xsl:output method="xml" omit-xml-declaration="yes" indent="yes"/>

  <xsl:template match="/Project" >
    <html>
      <head>        
        <meta http-equiv="refresh" content="5" />
      </head>
      
      <style><![CDATA[
      html,
      body {
      width: 100%;
      height: 100%;
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
      overflow-x: auto;
      }

      .Project::-webkit-scrollbar {
      display: none;
      }

      .card--content {
      background-color: #e74c3c;
      min-width: 200px;
      margin: 5px;
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
