/***************************************************************************
                          ksteventmonitorentry.h  -  description
                             -------------------
    begin                : Tue Apr 6 2004
    copyright            : (C) 2000 by The University of British Columbia
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

#ifndef KSTEVENTMONITORENTRY_H
#define KSTEVENTMONITORENTRY_H

#include <qtimer.h>

#include "kstdataobject.h"
#include "kstdebug.h"

namespace Equation {
  class Node;
  class Context;
}

class EventMonitorEntry : public KstDataObject {
  Q_OBJECT
  public:
    EventMonitorEntry(const QString &in_tag);
    EventMonitorEntry(const QDomElement &e);
    virtual ~EventMonitorEntry();

    virtual UpdateType update(int updateCounter = -1);
    virtual void save(QTextStream &ts, const QString& indent = QString::null);
    virtual QString propertyString() const;
    virtual void _showDialog();

    bool needToEvaluate();
    bool isValid() const { return _bIsValid; }

    void log(int iIndex);
    const QString& event() const { return _strEvent; }
    const QString& description() const { return _strDescription; }
    KstDebug::LogLevel level() const { return _level; }
    Equation::Node* expression() const { return _pExpression; }
    bool logKstDebug() const { return _bLogKstDebug; }
    bool logEMail() const { return _bLogEMail; }
    bool logELOG() const { return _bLogELOG; }
    const QString& eMailRecipients() const { return _strEMailRecipients; }

    void setEvent(const QString& str);
    void setDescription(const QString& str);
    void setLevel(KstDebug::LogLevel level);
    void setExpression(Equation::Node* pExpression);
    void setLogKstDebug(bool bLogKstDebug);
    void setLogEMail(bool bLogEMail);
    void setLogELOG(bool bLogELOG);
    void setEMailRecipients(const QString& str);

    void logImmediately();

    bool reparse();
    
    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap);
    
    void replaceDependency(KstDataObjectPtr oldObject, KstDataObjectPtr newObject);

    void replaceDependency(KstVectorPtr oldVector, KstVectorPtr newVector);
    void replaceDependency(KstMatrixPtr oldMatrix, KstMatrixPtr newMatrix);

    bool uses(KstObjectPtr p) const;

  private slots:
    void  slotUpdate();

  private:
    void commonConstructor(const QString &in_tag);

    static const QString OUTXVECTOR;
    static const QString OUTYVECTOR;

    KstVectorMap        _vectorsUsed;
    QValueList<int>     _indexArray;
    QString             _strEvent;
    QString             _strDescription;
    QString             _strEMailRecipients;
    KstDebug::LogLevel  _level;
    Equation::Node*     _pExpression;
    KstVectorMap::Iterator _xVector;
    KstVectorMap::Iterator _yVector;
    bool                _bLogKstDebug;
    bool                _bLogEMail;
    bool                _bLogELOG;
    bool                _bIsValid;
    int                 _iNumDone;
};

typedef KstSharedPtr<EventMonitorEntry> EventMonitorEntryPtr;
typedef KstObjectList<EventMonitorEntryPtr> KstEventMonitorEntryList;

#endif
// vim: ts=2 sw=2 et
