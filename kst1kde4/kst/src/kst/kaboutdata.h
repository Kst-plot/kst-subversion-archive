/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
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

#ifndef KABOUTDATA_H
#define KABOUTDATA_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QSharedDataPointer>

template <class T> class QList;
class QVariant;
class KAboutData;

class KAboutPerson
{
    friend class KAboutData;
public:

    explicit KAboutPerson( const QString &name,
                           const QString &task = QString(),
                           const QByteArray &emailAddress = QByteArray(),
                           const QByteArray &webAddress = QByteArray() );

    KAboutPerson(const KAboutPerson& other);

    ~KAboutPerson();

    KAboutPerson& operator=(const KAboutPerson& other);

    QString name() const;

    QString task() const;

    QString emailAddress() const;

    QString webAddress() const;

private:
    explicit KAboutPerson( const QString &name, const QString &email );

    class Private;
    Private *const d;
};

class KAboutLicense;

class KAboutData
{
  public:

    enum LicenseKey
    {
      License_Custom = -2,
      License_File = -1,
      License_Unknown = 0,
      License_GPL  = 1,
      License_GPL_V2 = 1,
      License_LGPL = 2,
      License_LGPL_V2 = 2,
      License_BSD  = 3,
      License_Artistic = 4,
      License_QPL = 5,
      License_QPL_V1_0 = 5,
      License_GPL_V3 = 6,
      License_LGPL_V3 = 7
    };

    enum NameFormat
    {
        ShortName,
        FullName
    };

  public:
    KAboutData( const QByteArray &appName,
                const QByteArray &catalogName,
                const QString &programName,
                const QByteArray &version,
                const QString &shortDescription = QString(),
                enum LicenseKey licenseType = License_Unknown,
                const QString &copyrightStatement = QString(),
                const QString &otherText = QString(),
                const QByteArray &homePageAddress = QByteArray(),
                const QByteArray &bugsEmailAddress = "submit@bugs.kde.org"
              );

    KAboutData(const KAboutData& other);

    KAboutData& operator=(const KAboutData& other);

    ~KAboutData();

    KAboutData &addAuthor( const QString &name,
                           const QString &task = QString(),
                           const QByteArray &emailAddress = QByteArray(),
                           const QByteArray &webAddress = QByteArray() );

    KAboutData &addCredit( const QString &name,
                           const QString &task = QString(),
                           const QByteArray &emailAddress = QByteArray(),
                           const QByteArray &webAddress = QByteArray() );

    KAboutData &setTranslator( const QString& name,
                               const QString& emailAddress );


    KAboutData &setLicenseText( const QString &license );

    KAboutData &addLicenseText( const QString &license );

    KAboutData &setLicenseTextFile( const QString &file );

    KAboutData &addLicenseTextFile( const QString &file );

    KAboutData &setAppName( const QByteArray &appName );

    KAboutData &setProgramName( const QString &programName );

    KAboutData &setProgramIconName( const QString &iconName );

    KAboutData &setProgramLogo(const QVariant& image);

    KAboutData &setVersion( const QByteArray &version );

    KAboutData &setShortDescription( const QString &shortDescription );

    KAboutData &setCatalogName( const QByteArray &catalogName );

    KAboutData &setLicense( LicenseKey licenseKey );

    KAboutData &addLicense( LicenseKey licenseKey );

    KAboutData &setCopyrightStatement( const QString &copyrightStatement );

    KAboutData &setOtherText( const QString &otherText );

    KAboutData &setHomepage( const QByteArray &homepage );

    KAboutData &setBugAddress( const QByteArray &bugAddress );

    KAboutData &setOrganizationDomain( const QByteArray &domain );

    KAboutData &setProductName( const QByteArray &name );
 
    QString appName() const;

    QString productName() const;

    QString programName() const;

    QString organizationDomain() const;

    const char* internalProgramName() const;

    void translateInternalProgramName() const;

    QString programIconName() const;

    QVariant programLogo() const;

    QString version() const;

    const char* internalVersion() const;

    QString shortDescription() const;

    QString catalogName() const;

    QString homepage() const;

    QString bugAddress() const;

    const char* internalBugAddress() const;

    QList<KAboutPerson> authors() const;

    QList<KAboutPerson> credits() const;

    QList<KAboutPerson> translators() const;

    static QString aboutTranslationTeam();

    QString otherText() const;

    QString license() const;

    QString licenseName(NameFormat formatName) const;

		QList<KAboutLicense> licenses() const;

    QString copyrightStatement() const;

    QString customAuthorPlainText() const;

    QString customAuthorRichText() const;

    bool customAuthorTextEnabled() const;

    KAboutData &setCustomAuthorText(const QString &plainText,
                                    const QString &richText);

    KAboutData &unsetCustomAuthorText();

  private:

    class Private;
    Private *const d;
};


class KAboutLicense
{
    friend class KAboutData;
public:
    KAboutLicense(const KAboutLicense& other);

    ~KAboutLicense();

    KAboutLicense& operator=(const KAboutLicense& other);

    QString text() const;

    QString name(KAboutData::NameFormat formatName) const;

    KAboutData::LicenseKey key() const;

    static KAboutLicense byKeyword(const QString &keyword);

private:

    explicit KAboutLicense( enum KAboutData::LicenseKey licenseType, const KAboutData *aboutData );
 
// xxx    explicit KAboutLicense( const QString &pathToFile, const KAboutData *aboutData );

    explicit KAboutLicense( const QString &licenseText, const KAboutData *aboutData );

    class Private;
    QSharedDataPointer<Private> d;
};

#endif
