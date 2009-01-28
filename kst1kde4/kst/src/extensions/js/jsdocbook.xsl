<xsl:stylesheet version = "1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:param name="srcdir"/>
  <xsl:output method="text" encoding="iso-8859-1"/> 

  <xsl:template match="//classes">
  <xsl:text>#!/bin/sh</xsl:text>
#
  <xsl:for-each select="class">
    <xsl:value-of select="$srcdir"/>/jsdocs2xml.pl <xsl:value-of select="$srcdir"/>/<xsl:value-of select="@file"/>.h
  </xsl:for-each>

  echo \&lt;appendix id=\"script-chapter\"\&gt; &gt; script-chapter.docbook
  echo \&lt;title\&gt;Working with KstScript\&lt;/title\&gt; &gt;&gt; script-chapter.docbook
  xsltproc <xsl:value-of select="$srcdir"/>/jsdocbook-index.xsl     <xsl:value-of select="$srcdir"/>/classindex.xml &gt;&gt; script-chapter.docbook
  <xsl:for-each select="class">
    [ ! -e <xsl:value-of select="@file"/>.xml ] ||
    xsltproc <xsl:value-of select="$srcdir"/>/jsdocbook-class.xsl <xsl:value-of select="@file"/>.xml &gt;&gt; script-chapter.docbook
  </xsl:for-each>
  echo \&lt;/appendix\&gt; &gt;&gt; script-chapter.docbook
  </xsl:template>
</xsl:stylesheet>
