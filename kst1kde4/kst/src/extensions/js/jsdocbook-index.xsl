<xsl:stylesheet version = '1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
  <xsl:output method="xml" version="4.0" encoding="iso-8859-1" omit-xml-declaration="yes" indent="yes"/>

  <xsl:template match="//classes">
    <xsl:variable name="columns" select="3"/>
    <sect1><title id="kstscript_classes">KstScript Classes:</title>
      <informaltable frame="none">
	<tgroup cols="3" align="left" colsep="1" rowsep="1">
	  <colspec colname="c1"/>
	  <colspec colname="c2"/>
	  <colspec colname="c3"/>
	  <tbody>
            <xsl:for-each select="class[position() mod $columns = 1 or $columns = 1]">
              <row>
                <xsl:for-each select=". | following-sibling::class[position() &lt; $columns]">
                  <entry>
                    <link linkend="class_{@name}"><xsl:value-of select="@name"/></link>
                  </entry>
                </xsl:for-each>
              </row>
            </xsl:for-each>
          </tbody>
        </tgroup>
      </informaltable>
    </sect1>
  </xsl:template>
</xsl:stylesheet>
