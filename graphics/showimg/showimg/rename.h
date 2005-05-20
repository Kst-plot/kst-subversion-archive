/***************************************************************************
                         rename.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#ifndef RENAMESERIES_H
#define RENAMESERIES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
#include <qvariant.h>
#include <qspinbox.h>

// KDE
#include <kdatepicker.h>
#include <kdialogbase.h>

class BatchRenamer;
class ProgressDialog;

class QPopupMenu;

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QLabel;
class QSpinBox;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QButtonGroup;
class QRadioButton;
class KDateWidget;
class QFrame;
class QListView;
class QListViewItem;
class QToolButton;

class KCompletion;
class KLineEdit;
class KConfig;


class RenameSeries : public KDialogBase
{
    Q_OBJECT

public:
    /**
    	construct a dialog to rename in series files
    */
    RenameSeries( QWidget* parent = 0, const char* name = 0 );
    ~RenameSeries();

	void readConfig(KConfig* config, const QString& group);
	void writeConfig(KConfig* config, const QString& group);

    /**
    	@param filename a file to add for rename
    */
    void addFile(const QString& filename);

    /**
    	re-init all data
    */
    void clear();
    int exec ();

    void setDateFormat(const QString& format);
    QString getDateFormat();
    void setTimeFormat(const QString& format);
    QString getTimeFormat();

public slots:
    void slotOk();


protected:
    QString getPath( const QString& fn );
    bool checkErrors(bool checkDir=true);

    QPopupMenu *EXIFpopup;

    BatchRenamer *m_batchRenamer;
    ProgressDialog *m_progressDialog;

    QListViewItem *last;
    QMemArray<QString*> arrayNames;
    int taille;

    BatchRenamer *newNames;
    QListViewItem* currentItem;
    QPixmap *pix;

protected:
    QGroupBox* GroupBox1;
    QGroupBox* GroupBox9;
    QLabel* TextLabel2_2;
    QSpinBox* spinIndex;
    QGroupBox* groupBox4;
    QLineEdit* paternLineEdit;
    QPushButton* EXIFButton;
    QCheckBox* overwrite;
    QCheckBox* checkExtension;
    QButtonGroup* ButtonGroup1;
    QRadioButton* optionCopy;
    QRadioButton* optionMove;
    QLineEdit* dirname;
    QPushButton* buttonChooseDir;
    QRadioButton* optionRename;
    QGroupBox* GroupBox5;
    QCheckBox* CheckBox7;
    KDateWidget* kDatePicker;
    QFrame* Line2;
    QGroupBox* groupBox5;
    QListView* remanedPreviewListView;
    QToolButton* moveUpPushButton;
    QToolButton* moveDownPushButton;
    QCheckBox* previewCheckBox;
    QLabel* PixmapLabel;

    QGridLayout* RenameSeriesLayout;
    QGridLayout* GroupBox1Layout;
    QHBoxLayout* GroupBox9Layout;
    QSpacerItem* spacer12;
    QGridLayout* groupBox4Layout;
    QGridLayout* ButtonGroup1Layout;
    QVBoxLayout* GroupBox5Layout;
    QVBoxLayout* groupBox5Layout;
    QHBoxLayout* layout13;
    QVBoxLayout* layout12;
    QVBoxLayout* layout15;
    QHBoxLayout* layout14_3;
    QSpacerItem* spacer6_3;
    QSpacerItem* spacer4_3;


protected slots:
    virtual void languageChange();

    virtual void slotMoveDown();
    virtual void slotMoveUp();
    virtual void slotSetImagePreview(int);
    virtual void slotUpdatePreview(QListViewItem*);
    virtual void slotUpdateRenamed(const QString&);
    virtual void slotUpdateRenamed();

    void chooseDir();

    void EXIFButtonClicked();
    void EXIFpopupMenuClicked(int pos);
};

#endif
