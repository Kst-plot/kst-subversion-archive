/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2004 The University of Toronto

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KSTCOMBOBOX_H
#define _KSTCOMBOBOX_H

#include <qlineedit.h>
#include <qcombobox.h>

#include <kcompletion.h>

class QListBoxItem;
class QPopupMenu;
class QLineEdit;

class KCompletionBox;
class KURL;

/**
 * A combined button, line-edit and a popup list widget.
 *
 * \b Detail \n
 *
 * This widget inherits from QComboBox and implements the following
 * additional functionalities:  a completion object that provides both automatic
 * and manual text completion as well as text rotation features, configurable
 * key-bindings to activate these features, and a popup-menu item that can be
 * used to allow the user to change the text completion mode on the fly.
 *
 * To support these new features KstComboBox emits a few additional signals
 * such as completion( const QString& ) and textRotation( KeyBindgingType ).
 * The completion signal can be connected to a slot that will assist the user in
 * filling out the remaining text while the rotation signal can be used to traverse
 * through all possible matches whenever text completion results in multiple matches.
 * Additionally, a returnPressed() and a returnPressed( const QString& )
 * signals are emitted when the user presses the Enter/Return key.
 *
 * KCombobox by default creates a completion object when you invoke the
 * completionObject( bool ) member function for the first time or
 * explicitly use setCompletionObject( KCompletion*, bool ) to assign your
 * own completion object.  Additionally, to make this widget more functional,
 * KstComboBox will by default handle text rotation and completion events
 * internally whenever a completion object is created through either one of the
 * methods mentioned above.  If you do not need this functionality, simply use
 * KCompletionBase::setHandleSignals(bool) or alternatively set the boolean
 * parameter in the @p setCompletionObject call to false.
 *
 * The default key-bindings for completion and rotation is determined from the
 * global settings in KStdAccel. These values, however, can be overridden
 * locally by invoking KCompletionBase::setKeyBinding(). The values can
 * easily be reverted back to the default setting, by simply calling
 * useGlobalSettings(). An alternate method would be to default individual
 * key-bindings by usning setKeyBinding() with the default second argument.
 *
 * A non-editable combobox only has one completion mode, @p CompletionAuto.
 * Unlike an editable combobox the CompletionAuto mode, works by matching 
 * any typed key with the first letter of entries in the combobox. Please note
 * that if you call setEditable( false ) to change an editable combobox to a
 * non-editable one, the text completion object associated with the combobox will
 * no longer exist unless you created the completion object yourself and assigned
 * it to this widget or you called setAutoDeleteCompletionObject( false ). In other
 * words do not do the following:
 *
 * \code
 * KstComboBox* combo = new KCompletionBox(true, this, "mywidget");
 * KCompletion* comp = combo->completionObject();
 * combo->setEditable( false );
 * comp->clear(); // CRASH: completion object does not exist anymore. 
 * \endcode
 *
 *
 * A read-only KstComboBox will have the same background color as a 
 * disabled KstComboBox, but its foreground color will be the one used for
 * the read-write mode. This differs from QComboBox's implementation
 * and is done to give visual distinction between the three different modes: 
 * disabled, read-only, and read-write.
 * 
 * \b Usage \n
 *
 * To enable the basic completion feature:
 *
 * \code
 * KstComboBox *combo = new KstComboBox( true, this, "mywidget" );
 * KCompletion *comp = combo->completionObject();
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&)));
 *
 * // Provide the to be completed strings. Note that those are separate from the combo's
 * // contents.
 * comp->insertItems( someQStringList );
 * \endcode
 *
 * To use your own completion object:
 *
 * \code
 * KstComboBox *combo = new KstComboBox( this,"mywidget" );
 * KURLCompletion *comp = new KURLCompletion();
 * combo->setCompletionObject( comp );
 * // Connect to the return pressed signal - optional
 * connect(combo,SIGNAL(returnPressed(const QString&)),comp,SLOT(addItem(const QString&)));
 * \endcode
 *
 * Note that you have to either delete the allocated completion object
 * when you don't need it anymore, or call
 * setAutoDeleteCompletionObject( true );
 *
 * Miscellaneous function calls:
 *
 * \code
 * // Tell the widget not to handle completion and rotation
 * combo->setHandleSignals( false );
 * // Set your own completion key for manual completions.
 * combo->setKeyBinding( KCompletionBase::TextCompletion, Qt::End );
 * // Hide the context (popup) menu
 * combo->setContextMenuEnabled( false );
 * \endcode
 *
 * @short An enhanced combo box.
 * @author Dawit Alemayehu <adawit@kde.org>
 */
 
#include "kst_export.h"
 
class KstComboBox : public QComboBox, public KCompletionBase
{
  Q_OBJECT
  Q_PROPERTY( bool autoCompletion READ autoCompletion WRITE setAutoCompletion )
  Q_PROPERTY( bool contextMenuEnabled READ isContextMenuEnabled WRITE setContextMenuEnabled )
  Q_PROPERTY( bool urlDropsEnabled READ isURLDropsEnabled WRITE setURLDropsEnabled )
  Q_PROPERTY( bool trapReturnKey READ trapReturnKey WRITE setTrapReturnKey )

public:

    /**
    * Constructs a read-only or rather select-only combo box with a
    * parent object and a name.
    *
    * @param parent The parent object of this widget
    * @param name The name of this widget
    */
    KST_EXPORT KstComboBox( QWidget *parent=0, const char *name=0 );

    /**
    * Constructs a "read-write" or "read-only" combo box depending on
    * the value of the first argument( @p rw ) with a parent, a
    * name.
    *
    * @param rw When @p true, widget will be editable.
    * @param parent The parent object of this widget.
    * @param name The name of this widget.
    */
    KST_EXPORT KstComboBox( bool rw, QWidget *parent=0, const char *name=0 );

    /**
    * Destructor.
    */
    virtual ~KstComboBox();

    /**
     * Sets @p url into the edit field of the combobox. It uses
     * KURL::prettyURL() so that the url is properly decoded for
     * displaying.
     */
    void setEditURL( const KURL& url );

    /**
     * Inserts @p url at position @p index into the combobox. The item will
     * be appended if @p index is negative. KURL::prettyURL() is used
     * so that the url is properly decoded for displaying.
     */
    void insertURL( const KURL& url, int index = -1 );

    /**
     * Inserts @p url with the pixmap &p pixmap at position @p index into
     * the combobox. The item will be appended if @p index is negative.
     * KURL::prettyURL() is used so that the url is properly decoded
     * for displaying.
     */
    void insertURL( const QPixmap& pixmap, const KURL& url, int index = -1 );

    /**
     * Replaces the item at position @p index with @p url.
     * KURL::prettyURL() is used so that the url is properly decoded
     * for displaying.
     */
    void changeURL( const KURL& url, int index );

    /**
     * Replaces the item at position @p index with @p url and pixmap @p pixmap.
     * KURL::prettyURL() is used so that the url is properly decoded
     * for displaying.
     */
    void changeURL( const QPixmap& pixmap, const KURL& url, int index );

    /**
    * Returns the current cursor position.
    *
    * This method always returns a -1 if the combo-box is @em not
    * editable (read-write).
    *
    * @return Current cursor position.
    */
    int cursorPosition() const { return ( lineEdit() ) ? lineEdit()->cursorPosition() : -1; }

    /**
    * Re-implemented from QComboBox.
    *
    * If @p true, the completion mode will be set to automatic.
    * Otherwise, it is defaulted to the global setting.  This
    * method has been replaced by the more comprehensive
    * setCompletionMode().
    *
    * @param autocomplete Flag to enable/disable automatic completion mode.
    */
    virtual void setAutoCompletion( bool autocomplete );

    /**
    * Re-implemented from QComboBox.
    *
    * Returns @p true if the current completion mode is set
    * to automatic.  See its more comprehensive replacement
    * completionMode().
    *
    * @return @p true when completion mode is automatic.
    */
    bool autoCompletion() const {
        return completionMode() == KGlobalSettings::CompletionAuto;
    }

    /**
    * Enables or disable the popup (context) menu.
    *
    * This method only works if this widget is editable, i.e.
    * read-write and allows you to enable/disable the context
    * menu.  It does nothing if invoked for a none-editable
    * combo-box.  Note that by default the mode changer item
    * is made visiable whenever the context menu is enabled.
    * Use hideModechanger() if you want to hide this
    * item.    Also by default, the context menu is created if
    * this widget is editable. Call this function with the
    * argument set to false to disable the popup menu.
    *
    * @param showMenu If @p true, show the context menu.
    */
    virtual void setContextMenuEnabled( bool showMenu );

    /**
    * Returns @p true when the context menu is enabled.
    */
    bool isContextMenuEnabled() const { return m_bEnableMenu; }

    /**
     * Enables/Disables handling of URL drops. If enabled and the user
     * drops an URL, the decoded URL will be inserted. Otherwise the default
     * behavior of QComboBox is used, which inserts the encoded URL.
     *
     * @param enable If @p true, insert decoded URLs
     */
    void setURLDropsEnabled( bool enable );

    /**
     * Returns @p true when decoded URL drops are enabled
     */
    bool isURLDropsEnabled() const;

    /**
     * Convenience method which iterates over all items and checks if
     * any of them is equal to @p text.
     *
     * If @p text is an empty string, @p false
     * is returned.
     *
     * @return @p true if an item with the string @p text is in the combobox.
     */
    bool contains( const QString& text ) const;

    /**
     * By default, KstComboBox recognizes Key_Return and Key_Enter
     * and emits the returnPressed() signals, but it also lets the 
     * event pass, for example causing a dialog's default-button to
     * be called.
     *
     * Call this method with @p trap equal to true to make KstComboBox
     * stop these events. The signals will still be emitted of course.
     *
     * Only affects read-writable comboboxes.
     *
     * @see setTrapReturnKey()
     */
    void setTrapReturnKey( bool trap );

    /**
     * @return @p true if keyevents of Key_Return or Key_Enter will
     * be stopped or if they will be propagated.
     *
     * @see setTrapReturnKey ()
     */
    bool trapReturnKey() const;

    /**
    * Re-implemented for internal reasons.  API not affected.
    */
    virtual bool eventFilter( QObject *, QEvent * );

    /**
     * @returns the completion-box, that is used in completion mode
     * KGlobalSettings::CompletionPopup and KGlobalSettings::CompletionPopupAuto.
     * This method will create a completion-box by calling
     * KLineEdit::completionBox, if none is there, yet.
     *
     * @param create Set this to false if you don't want the box to be created
     *               i.e. to test if it is available.
     */
    KCompletionBox * completionBox( bool create = true );

    /**
     * Re-implemented for internal reasons.  API remains unaffected.
     * NOTE: Only editable comboboxes can have a line editor. As such
     * any attempt to assign a line-edit to a non-editable combobox will
     * simply be ignored.
     */
    virtual void setLineEdit( QLineEdit * );

signals:
    /**
    * Emitted when the user presses the Enter key.
    *
    * Note that this signal is only emitted when the widget is editable.
    */
    void returnPressed();

    /**
    * Emitted when the user presses the Enter key.
    *
    * The argument is the current text being edited.  This signal is just like
    * returnPressed() except it contains the current text as its argument.
    *
    * Note that this signal is only emitted when the
    * widget is editable.
    */
    void returnPressed( const QString& );

    /**
    * Emitted when the completion key is pressed.
    *
    * The argument is the current text being edited.
    *
    * Note that this signal is @em not available when the widget is non-editable
    * or the completion mode is set to @p KGlobalSettings::CompletionNone.
    */
    void completion( const QString& );

    /**
     * Emitted when the shortcut for substring completion is pressed.
     */
    void substringCompletion( const QString& );

   /**
    * Emitted when the text rotation key-bindings are pressed.
    *
    * The argument indicates which key-binding was pressed. In this case this
    * can be either one of four values: @p PrevCompletionMatch,
    * @p NextCompletionMatch, @p RotateUp or @p RotateDown. See
    * KCompletionBase::setKeyBinding() for details.
    *
    * Note that this signal is @em NOT emitted if the completion
    * mode is set to CompletionNone.
    */
    void textRotation( KCompletionBase::KeyBindingType );

    /**
     * Emitted whenever the completion mode is changed by the user
     * through the context menu.
     */
    void completionModeChanged( KGlobalSettings::Completion );

    /**
     * Emitted before the context menu is displayed.
     *
     * The signal allows you to add your own entries into the context menu.
     * Note that you MUST NOT store the pointer to the QPopupMenu since it is
     * created and deleted on demand.  Otherwise, you can crash your app.
     *
     * @param p the context menu about to be displayed
     */
    void aboutToShowContextMenu( QPopupMenu * p );

public slots:

    /**
    * Iterates through all possible matches of the completed text
    * or the history list.
    *
    * Depending on the value of the argument, this function either
    * iterates through the history list of this widget or the all
    * possible matches in whenever multiple matches result from a
    * text completion request.  Note that the all-possible-match
    * iteration will not work if there are no previous matches, i.e.
    * no text has been completed and the *nix shell history list
    * rotation is only available if the insertion policy for this
    * widget is set either @p QComobBox::AtTop or @p QComboBox::AtBottom.
    * For other insertion modes whatever has been typed by the user
    * when the rotation event was initiated will be lost.
    *
    * @param type The key-binding invoked.
    */
    void rotateText( KCompletionBase::KeyBindingType type );

    /**
     * Sets the completed text in the line-edit appropriately.
     *
     * This function is an implementation for
     * KCompletionBase::setCompletedText.
     */
    virtual void setCompletedText( const QString& );

    /**
     * Sets @p items into the completion-box if completionMode() is
     * CompletionPopup. The popup will be shown immediately.
     */
    void setCompletedItems( const QStringList& items );

    /**
     * Selects the first item that matches @p item. If there is no such item,
     * it is inserted at position @p index if @p insert is true. Otherwise,
     * no item is selected.
     */
    void setCurrentItem( const QString& item, bool insert = false, int index = -1 );
    /**
     * Simply calls QComboBox' implementation. Only here to not become
     * shadowed.
     */
    void setCurrentItem(int index) { QComboBox::setCurrentItem(index); }

protected slots:

    /**
    * @deprecated
    */
    virtual void itemSelected( QListBoxItem* ) {};

    /**
    * Completes text according to the completion mode.
    *
    * Note: this method is @p not invoked if the completion mode is
    * set to CompletionNone.  Also if the mode is set to @p CompletionShell
    * and multiple matches are found, this method will complete the
    * text to the first match with a beep to inidicate that there are
    * more matches.  Then any successive completion key event iterates
    * through the remaining matches.  This way the rotation functionality
    * is left to iterate through the list as usual.
    */
    virtual void makeCompletion( const QString& );

protected:
    /*
    * This function simply sets the lineedit text and
    * highlights the text appropriately if the boolean
    * value is set to true.
    *
    * @param
    * @param
    */
    virtual void setCompletedText( const QString& /* */, bool /*marked*/ );

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void create( WId = 0, bool initializeWindow = true,
                         bool destroyOldWindow = true );

    virtual void wheelEvent( QWheelEvent *ev );

private slots:
    void lineEditDeleted();

private:
    /**
     * Initializes the variables upon construction.
     */
    void init();
    bool m_bEnableMenu; // ### BCI: unused, remove in KDE4
    bool m_trapReturnKey; // ### BCI: unused, remove in KDE4

private:
    class KstComboBoxPrivate;
    KstComboBoxPrivate *d;
};


#endif

