/***************************************************************************
                          batchrenamer.cpp  -  description
                             -------------------
    begin                : Sat Aug 18 2001
    copyright            : (C) 2001-2005 by Dominik Seichter
                                       Richard Groult
    email                : domseichter@web.de
                           rgroult@jalix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

// Local
#include "batchrenamer.h"

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#include <kdebug.h>
#include <kservice.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kprogress.h>

#include <qstringlist.h>


#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

BatchRenamer::BatchRenamer(KProgressDialog *p)
{
	this->p = p;
	p->setAutoClose(true);
	p->progressBar()->setProgress(-1);p->progressBar()->setTotalSteps(-1);

    KService* service=NULL;
    KService::List list = KService::allServices();
    for( unsigned int i = 0; i < list.count(); i++ ) {
        KService* s = (KService*)list[i];
        if( !s->terminal() && s->type() == QString::fromLatin1("Service") && s->hasServiceType( "KFilePlugin" ) )
	{
	    if(
	    	s->library() == QString::fromLatin1("kfile_jpeg")
		)
	    {
	     service=s;
	     KFileMetaInfoProvider* mip = KFileMetaInfoProvider::self();
	     QStringList options = service->serviceTypes();
	     for( unsigned int i = 0; i < options.count(); i++ )
	     {
	         if( options[i] != QString::fromLatin1("KFilePlugin") )
		 {
	             m_mimetype = options[i];
	             const KFileMimeTypeInfo* info = mip->mimeTypeInfo( m_mimetype );
	             if( info )
	                 keys = info->supportedKeys();

	             fileplugin = mip->plugin( m_mimetype );

		     KMimeType::Ptr mime = KMimeType::mimeType( m_mimetype );
		     setPattern( mime );
	         }
	     }

	    }
        }
    }

    m_klocale = KGlobal::locale();
    default_DateFormat=m_klocale->dateFormatShort();
    default_TimeFormat=m_klocale->timeFormat();
}

BatchRenamer::~BatchRenamer()
{
}

void BatchRenamer::processFiles( struct data* files, enum mode m, struct values* val, bool preview )
{
	QString tmp;
	int i;
	QFileInfo fi;
	QString text;

	for( i = 0; i < files[0].count; i++)
	{
		tmp = val->text;
		if( m == RENAME ) // final Path = source Path
			files[i].final_path = files[i].source_path;
		else
			files[i].final_path = val->dirname;

		files[i].source = doEscape( files[i].source );
		tmp = findBrackets( files[i].source,
			tmp,
			files[i].source_path + files[i].source + files[i].extension);

		tmp = findOldName( files[i].source, tmp );
		tmp = findOldNameLower( files[i].source, tmp );
		tmp = findOldNameUpper( files[i].source, tmp );
		tmp = findStar( files[i].source, tmp );
		tmp = findNumbers( tmp, val->index, files[0].count, i);

		// Add Extension if necesary
		files[i].final = unEscape( tmp );
		files[i].source = unEscape( files[i].source );
		if( val->extension )
		{
			if( !files[i].extension.isEmpty())
				files[i].final += files[i].extension;
		}
	}
	work( files, m, val, preview );
}


void
BatchRenamer::work( struct data * files, enum mode m, struct values * val, bool preview = false )
{
	int i, error = 0;
	QString in, out;
	QString errorMessages;
	m_renamedFiles.clear();
	m_renamedFiles.resize(files[0].count);

	for( i = 0; i < files[0].count; i++)
	{
	 	 in = files[i].source_path + files[i].source + files[i].extension;
	 	 out = files[i].final_path + files[i].final;

	 	 f = new QFile( out );
	 	 if( f->exists())
	 	 {
	 	 	 if(!val->overwrite)
	 	 	 {
	 	 	 	 error++;
	 	 	 	 delete f;
	 	 	 	 continue;
	 	 	 }
	 	 }
	 	 else
	 	 	delete f;


		 if(!preview)
		 {
			if(p) p->progressBar()->advance(1);

		 	 if( m == RENAME || m == MOVE )
		 	 {
// 				MYDEBUG<<"'"<<QFile::encodeName(in)<<"'->'"<< QFile::encodeName(out)<<"'"<<endl;
				if(!rename(QFile::encodeName(in), QFile::encodeName(out)))
				{
					m_renamedFiles.insert(QFile::encodeName(in), new QString(QFile::encodeName(out)));
				}
	 	 	 	else
	 	 	 	{
					errorMessages += i18n("Unable to rename %1 into %2. Error %3: %4").arg(QFile::encodeName(in)).arg(QFile::encodeName(out)).arg(errno).arg(sys_errlist[errno])+"\n";
	 	 	 		error++;
	 	 	 	}
	 	 	 }
	 	 	 else
	 	 	 if( m == COPY )
	 	 	 	if(!fcopy( in, out ))
	 	 	 		error++;

	 	 	 if( val->dvals.bDate )
	 	 	 {
	 	 		if(!changeDate( out, val->dvals ))
	 	 	 		error++;
	 	 	 }
	 	 }
	}
	if(error>0)
	{
		KMessageBox::detailedError(NULL, i18n("Unable to rename %1 files").arg(error), errorMessages, i18n("Error renaming files"));
	}
	if(p) p->close();
}

bool BatchRenamer::fcopy(const QString& src, const QString& dest )
{
	FILE* s;
	FILE* d;
	int c;

	d = fopen(QFile::encodeName(dest), "w");
	if( d == NULL )
	{
		return false;
	}

	s = fopen(QFile::encodeName(src), "r");
	if( s == NULL )
	{
		return false;
	}

	while(( c = getc( s )) != EOF )
		putc( c, d );

	fclose( s );
	fclose( d );
	return true;
}

int BatchRenamer::getCharacters( int n )
{
	QString s;
	s.sprintf( "%i", n );
	return s.length();
}


QString BatchRenamer::findNumbers( const QString& text, int index, int count, int i)
{
	// Rewritten in Version 0.8
	QString temp;
	QString _text_=text;
	int num, pos = 0, counter = 1;
	num = _text_.contains( "#", false );
	// Not really neccessary ;)
	if( num <= 0 )
		return _text_;

	pos = _text_.find("#", pos);
	pos++;
	while( _text_[pos] == '#' )
	{
		_text_ = _text_.remove(pos, 1);
		counter++;
	}

	pos = _text_.find("#", 0);
	if( pos >= 0 )
	{
	 	temp.sprintf("%0*i", counter, index + i );
	 	_text_ = _text_.replace( pos, 1, temp);
	}

	return findNumbers( _text_, index, count, i);
}

QString BatchRenamer::findOldName( const QString& oldname, const QString& text )
{
    /*
     * pos can here be -1 because
     * findRev is called with it as a
     * value !
     */
    int pos = -1;
    QString _text_=text;
    do {
        pos = _text_.findRev("$", pos);
        if( pos >= 0 )
            _text_.replace( pos, 1, oldname);
    } while( pos >= 0 );
    return _text_;
}

QString BatchRenamer::findOldNameLower( const QString& oldname, const QString& text )
{
    int pos = -1;
    QString _text_=text;
    do {
        pos = _text_.findRev("%", pos);
        if( pos >= 0 )
            _text_.replace( pos, 1, oldname.lower());
    } while( pos >= 0 );
    return _text_;
}

QString BatchRenamer::findOldNameUpper( const QString& oldname, const QString& text )
{
    int pos = -1;
    QString _text_=text;
    do {
        pos = _text_.findRev("&", pos);
        if( pos >= 0 )
            _text_.replace( pos, 1, oldname.upper());
    } while( pos >= 0 );
    return _text_;
}

QString BatchRenamer::findStar( const QString& oldname, const QString& text )
{
    int pos = -1;
    QString _text_=text;
    do {
        pos = _text_.findRev("*", pos);
        if( pos >= 0 ) {
            QString tmp = oldname.lower();
            if( tmp[0].isLetter() )
                tmp[0] = tmp[0].upper();

            for( unsigned int i = 1; i < tmp.length(); i++ )
                if( tmp[i+1].isLetter() && !tmp[i].isLetter() )
                    tmp[i+1] = tmp[i+1].upper();

            _text_.replace( pos, 1, tmp);
        }
    } while( pos >= 0 );
    return _text_;
}

bool BatchRenamer::changeDate( const QString& file, struct datevals dvals )
{
	FILE * f;
	struct utimbuf * t = new utimbuf();
	struct tm tmp;
	struct stat st;

	time_t ti;

	f = fopen(QFile::encodeName(file), "r");
	if( f == NULL )
	{
		return false;
	}

	fclose( f );

	tmp.tm_mday = dvals.date.day();
	tmp.tm_mon = dvals.date.month() - 1;
	tmp.tm_year = dvals.date.year() - 1900;

	tmp.tm_hour = dvals.hour;
	tmp.tm_min = dvals.minute;
	tmp.tm_sec = dvals.second;
	tmp.tm_isdst = -1;

	ti = mktime( &tmp );
	if( ti == -1 )
	{
		return false;
	}

	if( stat(QFile::encodeName(file), &st ) == -1 )
	{
		return false;
	}

	if(dvals.changeAccess)
		t->actime = ti;
	else
		t->actime = st.st_atime;

	if(dvals.changeModification)
		t->modtime = ti;
	else
		t->modtime = st.st_mtime;

	if(utime(QFile::encodeName(file), t ) != 0)
	{
		return false;
	}

	return true;
}

QString BatchRenamer::doEscape( const QString& text )
{
    QString _text_=text;
    _text_ = BatchRenamer::escape( _text_, "&", QChar( 60000 ) );
    _text_ = BatchRenamer::escape( _text_, "$", QChar( 60001 ) );
    _text_ = BatchRenamer::escape( _text_, "%", QChar( 60002 ) );
    _text_ = BatchRenamer::escape( _text_, "§", QChar( 60003 ) );
    _text_ = BatchRenamer::escape( _text_, "#", QChar( 60004 ) );
    _text_ = BatchRenamer::escape( _text_, "[", QChar( 60005 ) );
    _text_ = BatchRenamer::escape( _text_, "]", QChar( 60006 ) );

    return _text_;
}

QString BatchRenamer::unEscape( const QString& text )
{
    QString _text_=text;
    _text_ = BatchRenamer::escape( _text_, QChar( 60000 ), "&" );
    _text_ = BatchRenamer::escape( _text_, QChar( 60001 ), "$" );
    _text_ = BatchRenamer::escape( _text_, QChar( 60002 ), "%" );
    _text_ = BatchRenamer::escape( _text_, QChar( 60003 ), "§" );
    _text_ = BatchRenamer::escape( _text_, QChar( 60004 ), "#" );
    _text_ = BatchRenamer::escape( _text_, QChar( 60005 ), "[" );
    _text_ = BatchRenamer::escape( _text_, QChar( 60006 ), "]" );

    return _text_;
}

QString BatchRenamer::escape( const QString& text, const QString& token, const QString& sequence )
{
    /*
     * NEVER, ABSOLUTELY NEVER change pos = 0
     * to pos = -1, it won't work !
     * This bug took hours to find and was
     * a serious bug in 1.7.
     */

    int pos = 0;
    QString _text_=text;
    do {
       pos = _text_.find( token, pos );
       if( pos >= 0 )
           _text_.replace( pos, 1, sequence );
    } while ( pos >= 0 );
    return _text_;
}


QString BatchRenamer::findBrackets( const QString& oldname, const QString& text, const QString& filePath )
{
    /*
     * looks for a statement in brackets [ ]
     * and calls findToken() with this statement.
     */
    int num, pos = -1, a;
    QString token;

    if( text.contains("]", false) <= 0 || text.isEmpty() )
    {
	return text;
    }

    num = text.contains("[", false);
    if(num <= 0 )
    {
	return text;
    }

    pos = text.findRev("[", pos);
    a = text.find("]", pos );
    if( a < 0 && pos >= 0 )
    {
	return text;
    }

    if( pos < 0 && a >= 0 )
    {
	return text;
    }

    QString _text_=text;
    if( pos >= 0 && a >= 0 ) {
        token = _text_.mid( pos+1, (a-pos)-1 );

        // support [4-[length]]
        token = findBrackets( oldname, token, filePath );

        _text_.remove( pos, (a-pos)+1 );
        _text_.insert( pos, findToken(token, filePath));
    }
    return findBrackets( oldname, _text_, filePath );
}

QString BatchRenamer::findToken( const QString& token, const QString& filePath )
{
    enum conversion { LOWER, UPPER, MIXED, STAR, STRIP, NONE, EMPTY, NUMBER };
    unsigned int numwidth = 0;

    QString _token_=token;
    conversion c = EMPTY;
    if( !token.left(1).compare("$") )
        c = NONE;
    else if( !token.left(1).compare("%") )
        c = LOWER;
    else if( !token.left(1).compare("&") )
        c = UPPER;
    else if( !token.left(1).compare("") )
        c = MIXED;
    else if( !token.left(1).compare("*") )
        c = STAR;
    else if( !token.left(1).compare("\\") )
        c = STRIP;
    else if( !token.left(1).compare("#") ) {
        while( !_token_.left(1).compare("#") ) {
            _token_.remove( 0, 1 );
            ++numwidth;
        }

        c = NUMBER;
    }

    if( c != EMPTY && c != NUMBER )
        _token_.remove( 0, 1 );

    QString save = _token_;
    _token_ = processToken( _token_, filePath );

    switch( c ) {
        case LOWER:
            _token_ = _token_.lower();
            break;
        case UPPER:
            _token_ = _token_.upper();
            break;
        case MIXED:
            _token_ = _token_.lower();
            _token_.replace( 0, 1, _token_[0].upper());
            break;
        case STAR:
            _token_ = findStar( _token_, "*" );
            break;
        case STRIP:
            _token_ = _token_.stripWhiteSpace();
            break;
        case NUMBER:
            {
                bool b = false;
                int n = _token_.toInt( &b );
                if( b )
                    _token_ = _token_.sprintf("%0*i", numwidth, n );
            }
            break;
        default:
            break;
    }
    return doEscape( _token_ );
}


QString BatchRenamer::processToken( const QString& token, const QString& filePath)
{
    QString tmp;
        tmp = processFileToken( token, filePath );
        if( !tmp.isNull() )
            return tmp;

    return QString::null;
}


QString BatchRenamer::processFileToken(const QString& token, const QString& filePath)
{
     QString filename = filePath;
     QString _token_=token;
     _token_=getPattern()+_token_;

    _token_ = _token_.lower();
    for( unsigned int i = 0; i < keys.count(); i++ )
    {
	if( _token_.lower() == keys[i].lower() )
	{
            KFileMetaInfo meta( filename );
            if( meta.isValid() ) {
                QString k = keys[i];
                if( k.startsWith( getPattern() ) )
                    k = k.mid( getPattern().length(), k.length() - getPattern().length() );

                QString ret = meta.item( k ).string( true ).stripWhiteSpace();

		/*
		BEGIN RG contrib
		*/
		if(k.contains("date", false))
		{
			m_klocale->setDateFormatShort(default_DateFormat);
			QDate date = m_klocale->readDate(ret);
			if(date.isValid ())
			{
				m_klocale->setDateFormatShort(getDateFormat());
				ret = m_klocale->formatDate(date, true);
			}
		}
		else
		if(k.contains("time", false))
		{
			m_klocale->setTimeFormat(default_TimeFormat);
			QTime time = m_klocale->readTime(ret);
			if(time.isValid ())
			{
				m_klocale->setTimeFormat(getTimeFormat());
				ret = m_klocale->formatTime(time, true);
			}
		}
		/*
		END RG contrib
		*/

                //kapp->processEvents();
                return ret;
            }
        }
    }

    return QString::null;
}

const QString
BatchRenamer::getPattern() const
{
    return m_pattern;
}

void
BatchRenamer::setPattern( KMimeType::Ptr mime )
{
    QStringList pattern = mime->patterns();
    if( pattern.count() ) {
        m_pattern = pattern[0];
        if( m_pattern.startsWith( "*." ) )
            m_pattern = m_pattern.right( m_pattern.length() - 2 );
    }

    // TODO: REFACTOR
    // We need a pattern
    if( m_pattern.isEmpty() ) {
        int a = 0;
        a = m_name.find( "-" );
        if( a > -1 )
            m_pattern = m_name.left( a ).lower();
        else {
            a = m_pattern.find( " " );
            if( a > -1 )
                m_pattern = m_name.left( a ).lower();
            else
                m_pattern = m_name;
        }
    }

    setupKeys();
}

void
BatchRenamer::setupKeys()
{
    for( unsigned int i = 0; i < keys.count(); i++ )
    {
        keys[i] = getPattern() + keys[i];
    }
}

QStringList
BatchRenamer::getKeys()
{
    QStringList list;
    for( unsigned int i = 0; i < keys.count(); i++ )
    {
        list.append(keys[i].right(keys[i].length()-getPattern().length()));
    }
    list.sort();
    return list;
}



/*
BEGIN RG contrib
*/
void
BatchRenamer::setDateFormat(const QString& format)
{
	m_DateFormat=format;
}
QString
BatchRenamer::getDateFormat()
{
	return m_DateFormat;
}
void
BatchRenamer::setTimeFormat(const QString& format)
{
	m_TimeFormat=format;
}
QString
BatchRenamer::getTimeFormat()
{
	return m_TimeFormat;
}
/*
END RG contrib
*/
