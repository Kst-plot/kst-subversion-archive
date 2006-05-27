// vim: set tabstop=4 shiftwidth=4 noexpandtab
/*
Gwenview - A simple image viewer for KDE
Copyright 2000-2004 Aur�lien G�teau
 
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
*/


#ifndef BATCHMANIPULATOR_H
#define BATCHMANIPULATOR_H   

// Qt includes
#include <qwidget.h>

// KDE includes
#include <kurl.h>

// Local includes
#include "imageutils/orientation.h"
namespace Gwenview {

class BatchManipulatorPrivate;

class LIBGWENVIEW_EXPORT BatchManipulator : public QObject {
Q_OBJECT
public:
	BatchManipulator(QWidget* parent, const KURL::List&, ImageUtils::Orientation);
	~BatchManipulator();
	void apply();

public slots:
	void slotImageLoaded();

signals:
	void imageModified(const KURL& url);

private:
	BatchManipulatorPrivate* d;
};

} // namespace
#endif /* BATCHMANIPULATOR_H */

