<xsl:stylesheet version = '1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
  <xsl:output method="xml" version="4.0" encoding="iso-8859-1" omit-xml-declaration="yes" indent="yes"/>
  <xsl:template match="//class">
    <xsl:variable name="thisClass" select="@name"/>

    <sect1 id="class_{@name}">
    <title>Class: <xsl:value-of select="@name"/></title>
    <para>
      <xsl:variable name="descr" select="//description"/>
      <xsl:variable name="replaced">
        <xsl:call-template name="clean-description">
          <xsl:with-param name="text" select="$descr"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:value-of select="$replaced" disable-output-escaping="yes"/>
    </para>

    <xsl:if test="count(inherits) &gt; 0">
      <xsl:for-each select="inherits">
        <para>
          <emphasis role="bold">Inherits:</emphasis> 
          <link linkend="class_{@name}"><xsl:value-of select="@name"/></link>
        </para>
      </xsl:for-each>
    </xsl:if>

    <xsl:if test="count(collection) &gt; 0">
      <xsl:for-each select="collection">
        <para>
          <emphasis role="bold">Collection class:</emphasis> 
          <link linkend="class_{@name}"><xsl:value-of select="@name"/></link>
        </para>
      </xsl:for-each> 
    </xsl:if>

    <!-- List constructors -->
    <xsl:if test="count(constructor) &gt; 0">
      <sect2 id="constructors_{@name}">
      <title>Constructors:</title>
      <itemizedlist>
      <xsl:for-each select="constructor">
        <listitem><para>
        <link linkend="constructor_{$thisClass}_{string(position())}"><xsl:value-of select="$thisClass"/></link>
        <xsl:text> ( </xsl:text>
        <xsl:call-template name="displayArgumentsInline">
          <xsl:with-param name="typed" select="0"/>
        </xsl:call-template>
        <xsl:text> )</xsl:text>
        <xsl:if test="@obsolete = 'true'">
          <xsl:text> [Obsolete]</xsl:text>
        </xsl:if>
	</para></listitem>
      </xsl:for-each>
      </itemizedlist>
      </sect2>
    </xsl:if>

    <!-- List all the methods -->
    <xsl:if test="count(method) &gt; 0">
      <sect2 id="methods_{@name}">
      <title>Methods:</title>
      <itemizedlist>
      <xsl:for-each select="method">
        <listitem><para>
	<xsl:choose>
          <xsl:when test="@obsolete = 'true'">
            <xsl:value-of select="@name"/>
          </xsl:when>
          <xsl:otherwise>
	    <link linkend="method_{$thisClass}_{string(position())}"><xsl:value-of select="@name"/></link>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:text> ( </xsl:text>
        <xsl:call-template name="displayArgumentsInline">
          <xsl:with-param name="typed" select="0"/>
        </xsl:call-template>
        <xsl:text> )</xsl:text>
        <xsl:if test="@obsolete = 'true'">
          <xsl:text> [Obsolete]</xsl:text>
        </xsl:if>
	</para></listitem>
      </xsl:for-each>
      </itemizedlist>
      </sect2>
    </xsl:if>

    <!-- List all the properties -->
    <xsl:if test="count(property) &gt; 0">
      <sect2 id="properties_{@name}">
      <title>Properties:</title>
      <itemizedlist>
      <xsl:for-each select="property">
	<listitem><para>
        <xsl:choose>
          <xsl:when test="@obsolete = 'true'">
            <xsl:value-of select="@name"/><xsl:text> [Obsolete]</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <link linkend="property_{$thisClass}_{string(position())}"><xsl:value-of select="@name"/></link>
          </xsl:otherwise>
        </xsl:choose>
	</para></listitem>
      </xsl:for-each>
      </itemizedlist>
      </sect2>
    </xsl:if>

    <sect2><title></title>
      <para></para>

      <!-- Display details of constructors -->
      <xsl:if test="count(constructor) &gt; 0">
        <xsl:for-each select="constructor">
          <sect3 id="constructor_{$thisClass}_{string(position())}">
            <title>
              <xsl:value-of select="$thisClass"/><xsl:text> ( </xsl:text>
              <xsl:call-template name="displayArgumentsInline"/>
              <xsl:text> )</xsl:text>
              <xsl:if test="@obsolete = 'true'">
                <xsl:text> [Obsolete]</xsl:text>
              </xsl:if>
            </title>

            <para>
              <xsl:call-template name="displayArgumentsFull"/>
              <xsl:variable name="descr" select="description"/>
              <xsl:variable name="replaced">
                <xsl:call-template name="clean-description">
                  <xsl:with-param name="text" select="$descr"/>
                </xsl:call-template>
              </xsl:variable>
              <xsl:value-of select="$replaced" disable-output-escaping="yes"/>
              <xsl:call-template name="displayExceptions"/>
            </para>
          </sect3>
        </xsl:for-each>
      </xsl:if>

      <!-- Display details of methods -->
      <xsl:if test="count(method) &gt; 0">
        <xsl:for-each select="method">
          <sect3 id="method_{$thisClass}_{string(position())}">
            <title>
              <xsl:call-template name="displayReturnType"/>
              <xsl:value-of select="@name"/>
              <xsl:text> ( </xsl:text>
              <xsl:call-template name="displayArgumentsInline"/>
              <xsl:text> )</xsl:text>
              <xsl:if test="@obsolete = 'true'">
                <xsl:text> [Obsolete]</xsl:text>
              </xsl:if>
            </title>

            <para>
              <xsl:call-template name="displayArgumentsFull"/>
              <xsl:variable name="descr" select="description"/>
              <xsl:variable name="replaced">
                <xsl:call-template name="clean-description">
                  <xsl:with-param name="text" select="$descr"/>
                </xsl:call-template>
              </xsl:variable>
              <xsl:value-of select="$replaced" disable-output-escaping="yes"/>
              <xsl:call-template name="displayExceptions"/>
            </para>
          </sect3>
        </xsl:for-each>
      </xsl:if>

      <!-- Display details of properties -->
      <xsl:if test="count(property) &gt; 0">
        <xsl:for-each select="property">
          <sect3 id="property_{$thisClass}_{string(position())}">
            <title>
              <xsl:call-template name="displayReturnType"/>
              <xsl:text> </xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:if test="@readonly = 'true'">
                <xsl:text> [Read-Only]</xsl:text>
              </xsl:if>
              <xsl:if test="@obsolete = 'true'">
                <xsl:text> [Obsolete]</xsl:text>
              </xsl:if>
            </title>

            <para>
              <xsl:variable name="descr" select="description"/>
              <xsl:variable name="replaced">
                <xsl:call-template name="clean-description">
                  <xsl:with-param name="text" select="$descr"/>
                </xsl:call-template>
              </xsl:variable>
              <xsl:value-of select="$replaced" disable-output-escaping="yes"/>
            </para>
          </sect3>
        </xsl:for-each>
      </xsl:if>

      </sect2>

    </sect1>
  </xsl:template>

  <xsl:template name="displayReturnType">
    <xsl:choose>
      <xsl:when test="@type and not(@type = '')">
        <xsl:choose>
          <xsl:when test="@type = 'number' or @type = 'string' or @type = 'boolean' or @type = 'date' or @type = 'QWidget' or @type = 'StringArray' or @type = 'Array'">
            <xsl:value-of select="@type"/>
          </xsl:when>
          <xsl:otherwise>
            <link linkend="class_{@type}"><xsl:value-of select="@type"/></link>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:text> </xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>void </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="displayArgumentsInline">
    <xsl:param name="typed" select="1"/>
    <xsl:param name="arg" select="1"/>
    <xsl:if test="$arg &lt; count(argument) + 1">
      <xsl:for-each select="argument[$arg]">
        <xsl:if test="@optional = 'true'"> [</xsl:if>
        <xsl:if test="$arg &gt; 1">, </xsl:if>
        <xsl:if test="$typed = 1">
          <xsl:choose>
            <xsl:when test="@type = 'number' or @type = 'string' or @type = 'boolean' or @type = 'date' or @type = 'QWidget' or @type = 'StringArray' or @type = 'Array'">
              <xsl:value-of select="@type"/>
            </xsl:when>
            <xsl:otherwise>
              <link linkend="class_{@type}"><xsl:value-of select="@type"/></link>
            </xsl:otherwise>
          </xsl:choose>
          <xsl:text> </xsl:text>
        </xsl:if>
        <xsl:value-of select="@name"/>
        <xsl:for-each select="..">
          <xsl:call-template name="displayArgumentsInline">
            <xsl:with-param name="typed" select="$typed"/>
            <xsl:with-param name="arg" select="$arg + 1"/>
          </xsl:call-template>
        </xsl:for-each>
        <xsl:if test="@optional = 'true'">]</xsl:if>
      </xsl:for-each>
    </xsl:if>
  </xsl:template>

  <xsl:template name="displayArgumentsFull">
    <xsl:if test="count(argument) &gt; 0">
      <itemizedlist>
      <xsl:for-each select="argument">
        <listitem><para>
        <xsl:choose>
          <xsl:when test="@type = 'number' or @type = 'string' or @type = 'boolean' or @type = 'date' or @type = 'QWidget' or @type = 'StringArray' or @type = 'Array'">
            <xsl:value-of select="@type"/>
          </xsl:when>
          <xsl:otherwise>
            <link linkend="class_{@type}"><xsl:value-of select="@type"/></link>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:text> </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:if test="description">
          <xsl:text> - </xsl:text>
	    <xsl:variable name="descr" select="description"/>
      	    <xsl:variable name="replaced">
              <xsl:call-template name="clean-description">
                <xsl:with-param name="text" select="$descr"/>
              </xsl:call-template>
            </xsl:variable>
          <xsl:value-of select="$replaced" disable-output-escaping="yes"/>
        </xsl:if>
        <xsl:if test="@optional = 'true'">
          <xsl:text> [OPTIONAL]</xsl:text>
        </xsl:if>
        </para></listitem>
      </xsl:for-each>
      </itemizedlist>
    </xsl:if>
  </xsl:template>

  <xsl:template name="displayExceptions">
    <xsl:if test="count(exception) &gt; 0">
      <itemizedlist>
      <xsl:for-each select="exception">
        <listitem><para>
        Throws: <xsl:value-of select="@name"/>
        <xsl:if test="description">
          <xsl:text> - </xsl:text>
          <xsl:value-of select="description" disable-output-escaping="yes"/>
        </xsl:if>
        </para></listitem>
      </xsl:for-each>
      </itemizedlist>
    </xsl:if>
  </xsl:template>

  <xsl:template name="clean-description">
    <xsl:param name="text"/>
    <xsl:variable name="replaced">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$text"/>
        <xsl:with-param name="replace" select="'&lt;li&gt;'"/>
        <xsl:with-param name="with" select="'&lt;listitem&gt;&lt;para&gt;'"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced2">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced"/>
        <xsl:with-param name="replace" select="'&lt;/li&gt;'"/>
        <xsl:with-param name="with" select="'&lt;/para&gt;&lt;/listitem&gt;'"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced3">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced2"/>
        <xsl:with-param name="replace" select="'&lt;ul&gt;'"/>
        <xsl:with-param name="with" select="'&lt;itemizedlist&gt;'"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced4">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced3"/>
        <xsl:with-param name="replace" select="'&lt;/ul&gt;'"/>
        <xsl:with-param name="with" select="'&lt;/itemizedlist&gt;'"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced5">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced4"/>
        <xsl:with-param name="replace" select="'&lt;i&gt;'"/>
        <xsl:with-param name="with" select="'&lt;emphasis&gt;'"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced6">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced5"/>
        <xsl:with-param name="replace" select="'&lt;/i&gt;'"/>
        <xsl:with-param name="with" select="'&lt;/emphasis&gt;'"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced7">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced6"/>
        <xsl:with-param name="replace" select="'&lt;code&gt;&lt;pre&gt;'"/>
        <xsl:with-param name="with" select="'&lt;programlisting&gt;&lt;![CDATA['"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced8">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced7"/>
        <xsl:with-param name="replace" select="'&lt;/pre&gt;&lt;/code&gt;'"/>
        <xsl:with-param name="with" select="'&#93;&#93;&gt;&lt;/programlisting&gt;'"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced9">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced8"/>
        <xsl:with-param name="replace" select="'&lt;p&gt;'"/>
        <xsl:with-param name="with" select="''"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="replaced10">
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$replaced9"/>
        <xsl:with-param name="replace" select="'&lt;/p&gt;'"/>
        <xsl:with-param name="with" select="''"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:value-of select="$replaced10"/>
  </xsl:template>

  <xsl:template name="replace-string">
    <xsl:param name="text"/>
    <xsl:param name="replace"/>
    <xsl:param name="with"/>
    <xsl:choose>
      <xsl:when test="contains($text,$replace)">
        <xsl:value-of select="substring-before($text,$replace)"/>
        <xsl:value-of select="$with"/>
        <xsl:call-template name="replace-string">
          <xsl:with-param name="text" select="substring-after($text,$replace)"/>
          <xsl:with-param name="replace" select="$replace"/>
          <xsl:with-param name="with" select="$with"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$text"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
