/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qstyle.h>

bool CurveAppearanceWidget::showLines()
{
    return _showLines->isChecked();
}


bool CurveAppearanceWidget::showPoints()
{
    return _showPoints->isChecked();
}


void CurveAppearanceWidget::init()
{
    reset();
    QTimer::singleShot(0, this, SLOT(drawLine()));
}


void CurveAppearanceWidget::fillCombo()
{
    KstPoint tmppoint;
    QRect rect;
    int iCurrentItem;
  
    rect = _combo->style().querySubControlMetrics(
           QStyle::CC_ComboBox, 
           _combo, 
           QStyle::SC_ComboBoxEditField );
    rect.setLeft( rect.left() + 2 );
    rect.setRight( rect.right() - 2 );
    rect.setTop( rect.top() + 2 );
    rect.setBottom( rect.bottom() - 2 );
    
    /* fill the point type dialog with point types */
    QPixmap ppix( rect.width(), rect.height() );
    QPainter pp( &ppix );
  
    iCurrentItem = _combo->currentItem( );
    _combo->clear();
    pp.setPen(color());

    for (int ptype = 0; ptype <= KSTPOINT_MAXTYPE; ptype++) {
        pp.fillRect( pp.window(), QColor("white"));
        tmppoint.setType(ptype);
        tmppoint.draw( &pp, ppix.width()/2, ppix.height()/2, 600 );
        _combo->insertItem(ppix);
    }
    
    _combo->setCurrentItem( iCurrentItem );
}


void CurveAppearanceWidget::setColor( QColor c )
{
    _color->setColor(c);
    drawLine();
}


QColor CurveAppearanceWidget::color()
{
    return _color->color();
}


void CurveAppearanceWidget::drawLine()
{
    QPixmap pix(_label->contentsRect().size());
    QPainter p(&pix);
    KstPoint tmppoint;

    p.fillRect(p.window(), QColor("white"));
    p.setPen(color());
    
    if (_showLines->isChecked()) {
	p.drawLine(1,pix.height()/2,pix.width()-1, pix.height()/2);
    }
    
    if (_showPoints->isChecked()) {
	tmppoint.setType(_combo->currentItem());
	tmppoint.draw(&p, pix.width()/2, pix.height()/2, 600);
    }
    
    _label->setPixmap(pix);
}


int CurveAppearanceWidget::pointType()
{
    return _combo->currentItem();
}


void CurveAppearanceWidget::reset(QColor newColor)
{
    _showLines->setChecked(true);
    _showPoints->setChecked(false);
    _color->setColor(newColor);
    fillCombo();
    drawLine();
}


void CurveAppearanceWidget::setValue( bool hasLines, bool hasPoints, const QColor & c, int pointType )
{
    _showLines->setChecked(hasLines);
    _showPoints->setChecked(hasPoints);
    _color->setColor(c);
    fillCombo();
    _combo->setCurrentItem(pointType);
    drawLine();
}


void CurveAppearanceWidget::comboChanged()
{
    _showPoints->setChecked(true);
    drawLine();
}


void CurveAppearanceWidget::reset()
{
    reset(KstColorSequence::next());
}


void CurveAppearanceWidget::setUsePoints( bool usePoints )
{
    _showPoints->setEnabled(usePoints);
    _combo->setEnabled(usePoints);
    if (!usePoints && _showPoints->isChecked()) {
	_showPoints->setChecked(false);
	drawLine();
    }
}


void CurveAppearanceWidget::setMustUseLines( bool bMustUseLines )
{
    _showLines->setEnabled(!bMustUseLines);
    if (bMustUseLines && !_showLines->isChecked()) {
	_showLines->setChecked(true);
	drawLine();
    }
}


void CurveAppearanceWidget::redrawCombo()
{
   fillCombo();
}
