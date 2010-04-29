/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 * Copyright (C) 2006 Nicolas GOUTTE <goutte@kde.org>
 * Copyright (C) 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "kaboutdata.h"

#include <QtCore/QFile>
#include <QtCore/QTextIStream>
#include <QtCore/QSharedData>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QHash>

// -----------------------------------------------------------------------------
// Design notes:
//
// These classes deal with a lot of text, some of which needs to be
// marked for translation. Since at the time when these object and calls are
// made the translation catalogs are usually still not initialized, the
// translation has to be delayed. This is achieved by using QString
// for translatable strings. KLocalizedStrings are produced by ki18n* calls,
// instead of the more usuall i18n* calls which produce QString by trying to
// translate immediately.
//
// All the non-translatable string arguments to methods are taken QByteArray,
// all the translatable are QString. The getter methods always return
// proper QString: the non-translatable strings supplied by the code are
// treated with QString::fromUtf8(), those coming from the outside with
// QTextCodec::toUnicode(), and translatable strings are finalized to QStrings
// at the point of getter calls (i.e. delayed translation).
// -----------------------------------------------------------------------------

class KAboutPerson::Private
{
public:
   QString _name;
   QString _task;
   QString _emailAddress;
   QString _webAddress;
   QString _nameNoop;
};

KAboutPerson::KAboutPerson( const QString &_name,
                            const QString &_task,
                            const QByteArray &_emailAddress,
                            const QByteArray &_webAddress )
  : d(new Private)
{
   d->_name = _name;
   d->_task = _task;
   d->_emailAddress = QString::fromUtf8(_emailAddress);
   d->_webAddress = QString::fromUtf8(_webAddress);
}

KAboutPerson::KAboutPerson( const QString &_name, const QString &_email )
  : d(new Private)
{
   d->_nameNoop = _name;
   d->_emailAddress = _email;
}

KAboutPerson::KAboutPerson(const KAboutPerson& other): d(new Private)
{
    *d = *other.d;
}

KAboutPerson::~KAboutPerson()
{
   delete d;
}

QString
KAboutPerson::name() const
{
   if (!d->_nameNoop.isEmpty())
      return d->_nameNoop;
   return d->_name;
}

QString
KAboutPerson::task() const
{
   if (!d->_task.isEmpty())
      return d->_task;
   return QString();
}

QString
KAboutPerson::emailAddress() const
{
   return d->_emailAddress;
}


QString
KAboutPerson::webAddress() const
{
   return d->_webAddress;
}


KAboutPerson&
KAboutPerson::operator=(const KAboutPerson& other)
{
   *d = *other.d;
   return *this;
}



class KAboutLicense::Private : public QSharedData
{
public:
    Private( enum KAboutData::LicenseKey licenseType, const KAboutData *aboutData );
// xxx    Private( const QString &pathToFile, const KAboutData *aboutData );
    Private( const QString &licenseText, const KAboutData *aboutData );
    Private( const Private& other);
public:
    enum KAboutData::LicenseKey  _licenseKey;
    QString             _licenseText;
    QString                      _pathToLicenseTextFile;
    // needed for access to the possibly changing copyrightStatement()
    const KAboutData *           _aboutData;
};

KAboutLicense::Private::Private( enum KAboutData::LicenseKey licenseType, const KAboutData *aboutData )
  : QSharedData(),
    _licenseKey( licenseType ),
    _aboutData( aboutData )
{
}
/* xxx
KAboutLicense::Private::Private( const QString &pathToFile, const KAboutData *aboutData )
  : QSharedData(),
    _licenseKey( KAboutData::License_File ),
    _pathToLicenseTextFile( pathToFile ),
    _aboutData( aboutData )
{
}
*/
KAboutLicense::Private::Private( const QString &licenseText, const KAboutData *aboutData )
  : QSharedData(),
    _licenseKey( KAboutData::License_Custom ),
    _licenseText( licenseText ),
    _aboutData( aboutData )
{
}

KAboutLicense::Private::Private(const KAboutLicense::Private& other)
  : QSharedData(other),
    _licenseKey( other._licenseKey ),
    _licenseText( other._licenseText ),
    _pathToLicenseTextFile( other._pathToLicenseTextFile ),
    _aboutData( other._aboutData )
{}


KAboutLicense::KAboutLicense( enum KAboutData::LicenseKey licenseType, const KAboutData *aboutData )
  : d(new Private(licenseType,aboutData))
{
}
/* xxx
KAboutLicense::KAboutLicense( const QString &pathToFile, const KAboutData *aboutData )
  : d(new Private(pathToFile,aboutData))
{
}
*/
KAboutLicense::KAboutLicense( const QString &licenseText, const KAboutData *aboutData )
  : d(new Private(licenseText,aboutData))
{
}

KAboutLicense::KAboutLicense(const KAboutLicense& other)
  : d(other.d)
{
}

KAboutLicense::~KAboutLicense()
{}

QString
KAboutLicense::text() const
{
    QString result;

    const QString lineFeed( "\n\n" );

    if (d->_aboutData && !d->_aboutData->copyrightStatement().isEmpty()) {
        result = d->_aboutData->copyrightStatement() + lineFeed;
    }

    bool knownLicense = false;
    QString pathToFile;
    switch ( d->_licenseKey )
    {
    case KAboutData::License_File:
        pathToFile = d->_pathToLicenseTextFile;
        break;
/* xxx
    case KAboutData::License_GPL_V2:
        knownLicense = true;
        pathToFile = KStandardDirs::locate("data", "LICENSES/GPL_V2");
        break;
    case KAboutData::License_LGPL_V2:
        knownLicense = true;
        pathToFile = KStandardDirs::locate("data", "LICENSES/LGPL_V2");
        break;
    case KAboutData::License_BSD:
        knownLicense = true;
        pathToFile = KStandardDirs::locate("data", "LICENSES/BSD");
        break;
    case KAboutData::License_Artistic:
        knownLicense = true;
        pathToFile = KStandardDirs::locate("data", "LICENSES/ARTISTIC");
        break;
    case KAboutData::License_QPL_V1_0:
        knownLicense = true;
        pathToFile = KStandardDirs::locate("data", "LICENSES/QPL_V1.0");
        break;
    case KAboutData::License_GPL_V3:
        knownLicense = true;
        pathToFile = KStandardDirs::locate("data", "LICENSES/GPL_V3");
        break;
    case KAboutData::License_LGPL_V3:
        knownLicense = true;
        pathToFile = KStandardDirs::locate("data", "LICENSES/LGPL_V3");
        break;
*/
    case KAboutData::License_Custom:
        if (!d->_licenseText.isEmpty()) {
            result = d->_licenseText;
            break;
        }
        // fall through
    default:
        result += QObject::tr("No licensing terms for this program have been specified.\n"
                       "Please check the documentation or the source for any\n"
                       "licensing terms.\n");
    }

    if (knownLicense) {
        result += QObject::tr("This program is distributed under the terms of the %1.").arg(name(KAboutData::ShortName));
        if (!pathToFile.isEmpty()) {
            result += lineFeed;
        }
    }

    if (!pathToFile.isEmpty()) {
        QFile file(pathToFile);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream str(&file);
            result += str.readAll();
        }
    }

    return result;
}


QString
KAboutLicense::name(KAboutData::NameFormat formatName) const
{
    QString licenseShort;
    QString licenseFull;

    switch (d->_licenseKey) {
    case KAboutData::License_GPL_V2:
        licenseShort = QObject::tr("GPL v2", "@item license (short name)");
        licenseFull = QObject::tr("GNU General Public License Version 2", "@item license");
        break;
    case KAboutData::License_LGPL_V2:
        licenseShort = QObject::tr("LGPL v2","@item license (short name)");
        licenseFull = QObject::tr("GNU Lesser General Public License Version 2", "@item license");
        break;
    case KAboutData::License_BSD:
        licenseShort = QObject::tr("BSD License", "@item license (short name)");
        licenseFull = QObject::tr("BSD License", "@item license");
        break;
    case KAboutData::License_Artistic:
        licenseShort = QObject::tr("Artistic License", "@item license (short name)");
        licenseFull = QObject::tr("Artistic License", "@item license");
        break;
    case KAboutData::License_QPL_V1_0:
        licenseShort = QObject::tr("QPL v1.0","@item license (short name)");
        licenseFull = QObject::tr("Q Public License", "@item license");
        break;
    case KAboutData::License_GPL_V3:
        licenseShort = QObject::tr("GPL v3", "@item license (short name)");
        licenseFull = QObject::tr("GNU General Public License Version 3", "@item license");
        break;
    case KAboutData::License_LGPL_V3:
        licenseShort = QObject::tr("LGPL v3", "@item license (short name)");
        licenseFull = QObject::tr("GNU Lesser General Public License Version 3", "@item license");
        break;
    case KAboutData::License_Custom:
    case KAboutData::License_File:
        licenseShort = licenseFull = QObject::tr("Custom", "@item license");
        break;
    default:
        licenseShort = licenseFull = QObject::tr("Not specified", "@item license");
    }

    const QString result =
        (formatName == KAboutData::ShortName ) ? licenseShort :
        (formatName == KAboutData::FullName ) ?  licenseFull :
                                                 QString();

    return result;
}


KAboutLicense&
KAboutLicense::operator=(const KAboutLicense& other)
{
   d = other.d;
   return *this;
}

KAboutData::LicenseKey
KAboutLicense::key() const
{
    return d->_licenseKey;
}

KAboutLicense
KAboutLicense::byKeyword(const QString &rawKeyword)
{
    // Setup keyword->enum dictionary on first call.
    // Use normalized keywords, by the algorithm below.
    static QHash<QString, KAboutData::LicenseKey> ldict;
    if (ldict.isEmpty()) {
        ldict.insert("gpl", KAboutData::License_GPL);
        ldict.insert("gplv2", KAboutData::License_GPL_V2);
        ldict.insert("gplv2+", KAboutData::License_GPL_V2);
        ldict.insert("lgpl", KAboutData::License_LGPL);
        ldict.insert("lgplv2", KAboutData::License_LGPL_V2);
        ldict.insert("lgplv2+", KAboutData::License_LGPL_V2);
        ldict.insert("bsd", KAboutData::License_BSD);
        ldict.insert("artistic", KAboutData::License_Artistic);
        ldict.insert("qpl", KAboutData::License_QPL);
        ldict.insert("qplv1", KAboutData::License_QPL_V1_0);
        ldict.insert("qplv10", KAboutData::License_QPL_V1_0);
        ldict.insert("gplv3", KAboutData::License_GPL_V3);
        ldict.insert("gplv3+", KAboutData::License_GPL_V3);
        ldict.insert("lgplv3", KAboutData::License_LGPL_V3);
        ldict.insert("lgplv3+", KAboutData::License_LGPL_V3);
    }

    // Normalize keyword.
    QString keyword = rawKeyword;
    keyword = keyword.toLower();
    keyword.remove(' ');
    keyword.remove('.');

    KAboutData::LicenseKey license = ldict.value(keyword,
                                                 KAboutData::License_Custom);
    return KAboutLicense(license, 0);
}


class KAboutData::Private
{
public:
    Private()
        : customAuthorTextEnabled(false)
        {}
    QString _appName;
    QString _programName;
    QString _shortDescription;
    QString _catalogName;
    QString _copyrightStatement;
    QString _otherText;
    QString _homepageAddress;
    QList<KAboutPerson> _authorList;
    QList<KAboutPerson> _creditList;
    QList<KAboutLicense> _licenseList;
    QString translatorName;
    QString translatorEmail;
    QString productName;
    QString programIconName;
    QVariant programLogo;
    QString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;

    QString organizationDomain;

    // Everything dr.konqi needs, we store as utf-8, so we
    // can just give it a pointer, w/o any allocations.
    QByteArray _translatedProgramName; // ### I don't see it ever being translated, and I did not change that
    QByteArray _version;
    QByteArray _bugEmailAddress;
};


KAboutData::KAboutData( const QByteArray &_appName,
                        const QByteArray &_catalogName,
                        const QString &_programName,
                        const QByteArray &_version,
                        const QString &_shortDescription,
                        enum LicenseKey licenseType,
                        const QString &_copyrightStatement,
                        const QString &text,
                        const QByteArray &homePageAddress,
                        const QByteArray &bugsEmailAddress
                      )
  : d(new Private)
{
    d->_appName = QString::fromUtf8(_appName);
    int p = d->_appName.indexOf('/');
    if (p >= 0) {
        d->_appName = d->_appName.mid(p + 1);
    }

    d->_catalogName = _catalogName;
    d->_programName = _programName;
    if (!d->_programName.isEmpty()) // KComponentData("klauncher") gives empty program name
        d->_translatedProgramName = _programName.toUtf8();
    d->_version = _version;
    d->_shortDescription = _shortDescription;
    d->_licenseList.append(KAboutLicense(licenseType,this));
    d->_copyrightStatement = _copyrightStatement;
    d->_otherText = text;
    d->_homepageAddress = homePageAddress;
    d->_bugEmailAddress = bugsEmailAddress;

    if (d->_homepageAddress.contains("http://")) {
        int dot = d->_homepageAddress.indexOf('.');
        if (dot >= 0) {
            d->organizationDomain = d->_homepageAddress.mid(dot + 1);
            int slash = d->organizationDomain.indexOf('/');
            if (slash >= 0)
                d->organizationDomain.truncate(slash);
        }
        else {
            d->organizationDomain = "kde.org";
        }
    }
    else {
        d->organizationDomain = "kde.org";
    }
}

KAboutData::~KAboutData()
{
    delete d;
}

KAboutData::KAboutData(const KAboutData& other): d(new Private)
{
    *d = *other.d;
    QList<KAboutLicense>::iterator it = d->_licenseList.begin(), itEnd = d->_licenseList.end();
    for ( ; it != itEnd; ++it) {
        KAboutLicense& al = *it;
        al.d.detach();
        al.d->_aboutData = this;
    }
}

KAboutData&
KAboutData::operator=(const KAboutData& other)
{
    if (this != &other) {
        *d = *other.d;
        QList<KAboutLicense>::iterator it = d->_licenseList.begin(), itEnd = d->_licenseList.end();
        for ( ; it != itEnd; ++it) {
            KAboutLicense& al = *it;
            al.d.detach();
            al.d->_aboutData = this;
        }
    }
    return *this;
}

KAboutData &
KAboutData::addAuthor( const QString &name,
                       const QString &task,
                       const QByteArray &emailAddress,
                       const QByteArray &webAddress )
{
  d->_authorList.append(KAboutPerson(name,task,emailAddress,webAddress));
  return *this;
}

KAboutData &
KAboutData::addCredit( const QString &name,
                       const QString &task,
                       const QByteArray &emailAddress,
                       const QByteArray &webAddress )
{
  d->_creditList.append(KAboutPerson(name,task,emailAddress,webAddress));
  return *this;
}

KAboutData &
KAboutData::setTranslator( const QString& name,
                           const QString& emailAddress )
{
  d->translatorName = name;
  d->translatorEmail = emailAddress;
  return *this;
}

KAboutData &
KAboutData::setLicenseText( const QString &licenseText )
{
    d->_licenseList[0] = KAboutLicense(licenseText,this);
    return *this;
}

KAboutData &
KAboutData::addLicenseText( const QString &licenseText )
{

    KAboutLicense &firstLicense = d->_licenseList[0];
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == License_Unknown) {
        firstLicense = KAboutLicense(licenseText,this);
    } else {
        d->_licenseList.append(KAboutLicense(licenseText,this));
    }
    return *this;
}

KAboutData &
KAboutData::setLicenseTextFile( const QString &pathToFile )
{
    d->_licenseList[0] = KAboutLicense(pathToFile,this);
    return *this;
}

KAboutData &
KAboutData::addLicenseTextFile( const QString &pathToFile )
{
    KAboutLicense &firstLicense = d->_licenseList[0];
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == License_Unknown) {
        firstLicense = KAboutLicense(pathToFile,this);
    } else {
        d->_licenseList.append(KAboutLicense(pathToFile,this));
    }
    return *this;
}

KAboutData &
KAboutData::setAppName( const QByteArray &_appName )
{
  d->_appName = QString::fromUtf8(_appName);
  return *this;
}

KAboutData &
KAboutData::setProgramName( const QString &_programName )
{
  d->_programName = _programName;
  translateInternalProgramName();
  return *this;
}

KAboutData &
KAboutData::setVersion( const QByteArray &_version )
{
  d->_version = _version;
  return *this;
}

KAboutData &
KAboutData::setShortDescription( const QString &_shortDescription )
{
  d->_shortDescription = _shortDescription;
  return *this;
}

KAboutData &
KAboutData::setCatalogName( const QByteArray &_catalogName )
{
  d->_catalogName = _catalogName;
  return *this;
}

KAboutData &
KAboutData::setLicense( LicenseKey licenseKey)
{
    d->_licenseList[0] = KAboutLicense(licenseKey,this);
    return *this;
}

KAboutData &
KAboutData::addLicense( LicenseKey licenseKey)
{
    KAboutLicense &firstLicense = d->_licenseList[0];
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == License_Unknown) {
        firstLicense = KAboutLicense(licenseKey,this);
    } else {
        d->_licenseList.append(KAboutLicense(licenseKey,this));
    }
    return *this;
}

KAboutData &
KAboutData::setCopyrightStatement( const QString &_copyrightStatement )
{
  d->_copyrightStatement = _copyrightStatement;
  return *this;
}

KAboutData &
KAboutData::setOtherText( const QString &_otherText )
{
  d->_otherText = _otherText;
  return *this;
}

KAboutData &
KAboutData::setHomepage( const QByteArray &_homepage )
{
  d->_homepageAddress = QString::fromUtf8(_homepage);
  return *this;
}

KAboutData &
KAboutData::setBugAddress( const QByteArray &_bugAddress )
{
  d->_bugEmailAddress = _bugAddress;
  return *this;
}

KAboutData &
KAboutData::setOrganizationDomain( const QByteArray &domain )
{
  d->organizationDomain = QString::fromUtf8(domain);
  return *this;
}

KAboutData &
KAboutData::setProductName( const QByteArray &_productName )
{
  d->productName = QString::fromUtf8(_productName);
  return *this;
}

QString
KAboutData::appName() const
{
   return d->_appName;
}

QString
KAboutData::productName() const
{
   if (!d->productName.isEmpty())
      return d->productName;
   return appName();
}

QString
KAboutData::programName() const
{
   if (!d->_programName.isEmpty())
      return d->_programName;
   return QString();
}

/// @internal
/// Return the program name. It is always pre-allocated.
/// Needed for KCrash in particular.
const char*
KAboutData::internalProgramName() const
{
   return d->_translatedProgramName.constData();
}

/// @internal
/// KCrash should call as few things as possible and should avoid e.g. malloc()
/// because it may deadlock. Since i18n() needs it, when KLocale is available
/// the i18n() call will be done here in advance.
void
KAboutData::translateInternalProgramName() const
{
  d->_translatedProgramName.clear();
/* xxx
  if( KGlobal::locale())
      d->_translatedProgramName = programName().toUtf8();
*/
}

QString
KAboutData::programIconName() const
{
    return d->programIconName.isEmpty() ? appName() : d->programIconName;
}

KAboutData &
KAboutData::setProgramIconName( const QString &iconName )
{
    d->programIconName = iconName;
    return *this;
}

QVariant
KAboutData::programLogo() const
{
    return d->programLogo;
}

KAboutData &
KAboutData::setProgramLogo(const QVariant& image)
{
    d->programLogo = image ;
    return *this;
}

QString
KAboutData::version() const
{
   return QString::fromUtf8(d->_version);
}

/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the version information. Used in particular for KCrash.
const char*
KAboutData::internalVersion() const
{
   return d->_version.constData();
}

QString
KAboutData::shortDescription() const
{
   if (!d->_shortDescription.isEmpty())
      return d->_shortDescription;
   return QString();
}

QString
KAboutData::catalogName() const
{
   if (!d->_catalogName.isEmpty())
      return d->_catalogName;
   // Fallback to appname for catalog name if empty.
   return d->_appName;
}

QString
KAboutData::homepage() const
{
   return d->_homepageAddress;
}

QString
KAboutData::bugAddress() const
{
   return QString::fromUtf8(d->_bugEmailAddress);
}

QString
KAboutData::organizationDomain() const
{
    return d->organizationDomain;
}


/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the bug mail address. Used in particular for KCrash.
const char*
KAboutData::internalBugAddress() const
{
   if (d->_bugEmailAddress.isEmpty())
      return 0;
   return d->_bugEmailAddress.constData();
}

QList<KAboutPerson>
KAboutData::authors() const
{
   return d->_authorList;
}

QList<KAboutPerson>
KAboutData::credits() const
{
   return d->_creditList;
}

#define NAME_OF_TRANSLATORS "Your names"
#define EMAIL_OF_TRANSLATORS "Your emails"
QList<KAboutPerson>
KAboutData::translators() const
{
    QList<KAboutPerson> personList;
/* xxx
    KLocale *tmpLocale = NULL;
    if (KGlobal::locale()) {
        // There could be many catalogs loaded into the global locale,
        // e.g. in systemsettings. The tmp locale is needed to make sure we
        // use the translators name from this aboutdata's catalog, rather than
        // from any other loaded catalog.
        tmpLocale = new KLocale(*KGlobal::locale());
        tmpLocale->setActiveCatalog(catalogName());
    }
*/
    QString translatorName;
    if (!d->translatorName.isEmpty()) {
        translatorName = d->translatorName;
    } else {
// xxx        translatorName = ki18nc("NAME OF TRANSLATORS", NAME_OF_TRANSLATORS).toString(tmpLocale);
    }

    QString translatorEmail;
    if (!d->translatorEmail.isEmpty()) {
        translatorEmail = d->translatorEmail;
    } else {
// xxx        translatorEmail = ki18nc("EMAIL OF TRANSLATORS", EMAIL_OF_TRANSLATORS).toString(tmpLocale);
    }

// xxx    delete tmpLocale;

    if ( translatorName.isEmpty() || translatorName == QString::fromUtf8( NAME_OF_TRANSLATORS ) )
        return personList;

    const QStringList nameList ( translatorName.split( ',' ) );

    QStringList emailList;
    if( !translatorEmail.isEmpty() && translatorEmail != QString::fromUtf8( EMAIL_OF_TRANSLATORS ) )
    {
       emailList = translatorEmail.split( ',', QString::KeepEmptyParts );
    }

    QStringList::const_iterator nit;
    QStringList::const_iterator eit = emailList.constBegin();

    for( nit = nameList.constBegin(); nit != nameList.constEnd(); ++nit )
    {
        QString email;
        if ( eit != emailList.constEnd() )
        {
            email = *eit;
            ++eit;
        }

// xxx        personList.append( KAboutPerson( (*nit).trimmed(), email.trimmed() ) );
    }

    return personList;
}

QString
KAboutData::aboutTranslationTeam()
{
    return QObject::tr("replace this with information about your translation team",
            "<p>KDE is translated into many languages thanks to the work "
            "of the translation teams all over the world.</p>"
            "<p>For more information on KDE internationalization "
            "visit <a href=\"http://l10n.kde.org\">http://l10n.kde.org</a></p>"
            ); }

QString
KAboutData::otherText() const
{
   if (!d->_otherText.isEmpty())
      return d->_otherText;
   return QString();
}

QString
KAboutData::license() const
{
    return d->_licenseList.at(0).text();
}

QString
KAboutData::licenseName(NameFormat formatName) const
{
    return d->_licenseList.at(0).name(formatName);
}

QList<KAboutLicense>
KAboutData::licenses() const
{
    return d->_licenseList;
}

QString
KAboutData::copyrightStatement() const
{
  if (!d->_copyrightStatement.isEmpty())
    return d->_copyrightStatement;
  return QString();
}

QString
KAboutData::customAuthorPlainText() const
{
  if (!d->customAuthorPlainText.isEmpty())
    return d->customAuthorPlainText;
  return QString();
}

QString
KAboutData::customAuthorRichText() const
{
  if (!d->customAuthorRichText.isEmpty())
    return d->customAuthorRichText;
  return QString();
}

bool
KAboutData::customAuthorTextEnabled() const
{
  return d->customAuthorTextEnabled;
}

KAboutData &
KAboutData::setCustomAuthorText(const QString &plainText,
                                const QString &richText)
{
  d->customAuthorPlainText = plainText;
  d->customAuthorRichText = richText;

  d->customAuthorTextEnabled = true;

  return *this;
}

KAboutData &
KAboutData::unsetCustomAuthorText()
{
  d->customAuthorPlainText = QString();
  d->customAuthorRichText = QString();

  d->customAuthorTextEnabled = false;

  return *this;
}

