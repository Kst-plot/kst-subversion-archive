/***************************************************************************
                          tools.h  -  description
                             -------------------
    begin                : Jan 22 2005
    copyright            : (C) 2005 by Richard Groult
    email                : rgroult@jalix.org
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

#ifndef TOOLS_H
#define TOOLS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>
#include <qimage.h>

class ImageSimilarityData;
class MainWindow;
class FormatConversion;
class RenameSeries;

class QFile;

class KAction;
class KActionCollection;
class KConfig;
class KProcess;
class KScanDialog;

class Tools : public QObject
{
	Q_OBJECT

public:
	Tools(MainWindow *mw );
	virtual ~Tools();

	void initActions(KActionCollection *actionCollection);

	void readConfig(KConfig *config);
	void writeConfig(KConfig *config);

	QString getConvertPath();
	void setConvertPath(const QString& path);

	QString getJpegtranPath();
	void setJpegtranPath(const QString& path);

	static bool saveAs(const QImage* image, const QString& oldPath, const QString& newPath);

public slots:
	void slotScanImage();

	void toolsRotateLeft();
	void toolsRotateRight();

	void renameSeries();

	void compareAlmost();
	void compareFast ();

	void convert();

private slots:
	void slotScanned(const QImage& img, int);
	void slotEndConvert(KProcess *proc);

private:
	MainWindow *mw;

	FormatConversion *formatConver;
	RenameSeries *m_renameS;

	QString m_convertPath, m_jpegtranPath;

#ifndef SHOWIMG_NO_SCAN_IMAGE
	KScanDialog *m_scanDialog;
#endif

KAction
			*aRenameSeries,
			*aToolsRotateLeft, *aToolsRotateRight,
			*aCompareFast,*aCompareAlmost,*aConvert
#ifndef SHOWIMG_NO_SCAN_IMAGE
			,*aScan
#endif
			;

private:
	char getRed(QImage* im, int x, int y);
	char getGreen(QImage* im, int x, int y);
	char getBlue(QImage* im, int x, int y);

	bool equals (QFile *f1, QFile *f2);


	ImageSimilarityData* image_sim_fill_data(const QString& filename);
	float image_sim_compare(ImageSimilarityData *a, ImageSimilarityData *b);
	float image_sim_compare_fast(ImageSimilarityData *a, ImageSimilarityData *b, float min);


};

#endif /* TOOLS_H */
