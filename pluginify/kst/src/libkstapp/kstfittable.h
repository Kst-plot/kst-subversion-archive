/***************************************************************************
                          kstfittable.h  -  description
                             -------------------
    begin                : Thu Mar 4 2004
    copyright            : (C) 2004 The University of British Columbia 
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTFITTABLE_H
#define KSTFITTABLE_H

class KstFitTable : public QTable
{
public:
    KstFitTable( QWidget * parent = 0, const char * name = 0 );
   ~KstFitTable( );

    void          setParameters(double* pParams, int iNumParams, double* pCovars, int iNumCovars, double dChi2Nu);
    void          paintCell( QPainter * p, int row, int col, const QRect & cr, bool selected, const QColorGroup & cg );

    void          resizeData(int) {}
    QWidget*      createEditor(int, int, bool) const { return 0; }
    QTableItem*   item(int, int) { return 0; }
    void          setItem(int, int, QTableItem *) {}
    void          clearCell(int, int) {}
    void          insertWidget(int, int, QWidget *) {}
    QWidget*      cellWidget(int, int) const { return 0; }
    void          clearCellWidget(int, int) {}

private:
    double*       _pdParams;
    double*       _pdCovars;
    double        _dChi2Nu;
    int           _iNumParams;
    int           _iNumCovars;
};

#endif
// vim: ts=2 sw=2 et
