/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "SurveyCount.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <klocale.h>
#include "DB/ImageDB.h"

using namespace Survey;

SurveyCountQuestion::SurveyCountQuestion( const QString& id, const QString& title, Survey::SurveyDialog* parent )
    :Question( id, title, parent )
{
    QVBoxLayout* vbox = new QVBoxLayout( this, 6 );
    QGridLayout* lay = new QGridLayout( vbox, 2, 2, 6 );
    QLabel* label = new QLabel( i18n("How many images are in your database:"), this );
    _imageCount = new QSpinBox( 0, 1000000, 100, this );
    lay->addWidget( label, 0, 0 );
    lay->addWidget( _imageCount, 0, 1 );
    _imageCount->setValue( DB::ImageDB::instance()->totalCount() );

    label = new QLabel(i18n("How many percentages of your images are scanned in: "), this );
    _scanned = new QSpinBox( 0, 100, 10, this );
    _scanned->setSpecialValueText( i18n("None" ) );
    _scanned->setSuffix(QString::fromLatin1( "%" ) );
    lay->addWidget( label, 1, 0 );
    lay->addWidget( _scanned, 1, 1 );

    vbox->addStretch( 1 );
}

void SurveyCountQuestion::save( QDomElement& elm )
{
    elm.setAttribute( QString::fromLatin1( "imageCount" ), _imageCount->value() );
    elm.setAttribute( QString::fromLatin1( "scannedPercentage" ), _scanned->value() );
}

void SurveyCountQuestion::load( QDomElement& elm )
{
    _imageCount->setValue( elm.attribute( QString::fromLatin1( "imageCount" ), QString::fromLatin1( "-1" ) ).toInt() );
    _scanned->setValue( elm.attribute( QString::fromLatin1( "scannedPercentage" ), QString::fromLatin1( "0" ) ).toInt() );
}
