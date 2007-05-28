/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VECTORMODEL_H
#define VECTORMODEL_H

#include <QAbstractItemModel>
#include <kstvector.h>

namespace Kst {

class VectorModel : public QAbstractItemModel
{
public:
  VectorModel(KstVectorPtr v);
  ~VectorModel();

  int columnCount(const QModelIndex& parent = QModelIndex()) const;
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int col, const QModelIndex& parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex& index) const;

private:
  KstVectorPtr _v;
};

}

#endif

// vim: ts=2 sw=2 et
