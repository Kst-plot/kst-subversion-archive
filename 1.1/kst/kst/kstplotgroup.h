/***************************************************************************
                               kstplotgroup.h
                             -------------------
    begin                : Mar 21, 2004
    copyright            : (C) 2004 The University of Toronto
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

#ifndef KSTPLOTGROUP_H
#define KSTPLOTGROUP_H


class KstPlotGroup : public KstMetaPlot {
  Q_OBJECT
  public:
    KstPlotGroup();
    KstPlotGroup(const QDomElement& e);
    KstPlotGroup(const KstPlotGroup& plotGroup);
    virtual ~KstPlotGroup();

    virtual void save(QTextStream& ts, const QString& indent = QString::null);
    virtual void saveTag(QTextStream& ts, const QString& indent = QString::null);

    virtual bool removeChild(KstViewObjectPtr obj, bool recursive = false);
    virtual bool popupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);
    virtual bool layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topParent);
    virtual void setHasFocus(bool hasFocus);
    virtual void removeFocus(QPainter& p);

  public slots:
    virtual void paint(KstPaintType type, QPainter& p);
    virtual void copyObject();
    virtual void copyObjectQuietly(KstViewObject& parent, const QString& name = QString::null) const;
    void flatten();

  protected:
    virtual KstViewObjectFactoryMethod factory() const;
};

typedef KstSharedPtr<KstPlotGroup> KstPlotGroupPtr;
typedef KstObjectList<KstPlotGroupPtr> KstPlotGroupList;


#endif
// vim: ts=2 sw=2 et
