/***************************************************************************
                                d2asc.cpp
                             -------------------
    begin                : Tue Aug 22 13:46:13 CST 2000
    copyright            : (C) 2000 by Barth Netterfield
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

#include <stdlib.h>
#include <QSettings>
#include <kinstance.h>

// hack to make main() a friend of kstdatasource
#define protected public
#include "kstrvector.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#undef protected

struct fieldEntry {
  QString field;
  bool doHex;
};

void Usage() {
  fprintf(stderr, "usage: d2asc filename [-f <first frame>]\n");
  fprintf(stderr, "             [-n <numframes>] [-s skip [-a]] \n");
  fprintf(stderr, "             [-x] col1 [[-x] col2 ... [-x] coln]\n");
  fprintf(stderr, "   -x specifies that the field should be printed in hex\n");
}


static void exitHelper() {
  KST::vectorList.clear();
  KST::scalarList.clear();
  KST::dataObjectList.clear();
}

int main(int argc, char *argv[]) {
  atexit(exitHelper);
  KInstance inst("d2asc");
  KstDataSourcePtr file;

  QSettings *qSettingsObject = new QSettings("kstdatarc", QSettings::NativeFormat, false);
  KstDataSource::setupOnStartup(qSettingsObject);

  fieldEntry field;
  QValueList<fieldEntry> fieldList;
  char *filename;
  bool do_ave = false;
  bool do_skip = false;
  int start_frame = 0;
  int n_frames = 2000000;
  int n_skip = 0;
  int NS = 0;
  int i_S;
  int i;

  if (argc < 3 || argv[1][0] == '-') {
    Usage();
    return -1;
  }

  filename = argv[1];

  for (i = 2; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'f') {
        i++;
        start_frame = atoi(argv[i]);
      } else if (argv[i][1] == 'n') {
        i++;
        n_frames = atoi(argv[i]);
      } else if (argv[i][1] == 's') {
        i++;
        n_skip = atoi(argv[i]);
        if (n_skip>0) do_skip = true;
      } else if (argv[i][1] == 'a') {
        do_ave = true;
      } else if (argv[i][1] == 'x') {
        i++;
        field.field = argv[i];
        field.doHex = true;
        fieldList.append(field);
      } else {
        Usage();
      }
    } else {
      field.field = argv[i];
      field.doHex = false;
      fieldList.append(field);
    }
  }

  if (!do_skip) {
    do_ave = false;
  }

  file = KstDataSource::loadSource(filename);
  if (!file || !file->isValid() || file->isEmpty()) {
    fprintf(stderr, "d2asc error: file %s has no data\n", filename);
    return -2;
  }
  /** make vectors and fill the list **/
  QPtrList<KstRVector> vlist;

  for (i=0; i<int(fieldList.size()); i++) {
    if (!file->isValidField(fieldList[i].field)) {
      fprintf(stderr, "d2asc error: field %s in file %s is not valid\n",
              fieldList[i].field.latin1(), filename);
      return -3;
    }
    KstRVectorPtr v = new KstRVector(file, fieldList[i].field, KstObjectTag("tag", KstObjectTag::globalTagContext), start_frame, n_frames, n_skip, n_skip>0, do_ave);
    vlist.append(v);
  }

  /* find NS */
  for (i = 0; i < int(fieldList.size()); i++) {
    vlist.at(i)->writeLock();
    while (vlist.at(i)->update(-1) != KstObject::NO_CHANGE); // read vector
    vlist.at(i)->unlock();
    if (vlist.at(i)->length() > NS) {
      NS = vlist.at(i)->length();
    }
  }

  for (i_S = 0; i_S < NS; i_S++) {
    for (i = 0; i < int(fieldList.size()); i++) {
      if (fieldList[i].doHex) {
        printf("%4x ",  (int)vlist.at(i)->interpolate(i_S, NS));
      } else {
        printf("%.16g ", vlist.at(i)->interpolate(i_S, NS));
      }
    }
    printf("\n");
  }
}
