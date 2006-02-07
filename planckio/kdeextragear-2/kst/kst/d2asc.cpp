#include <stdlib.h>
#include <iostream>

#include <qptrlist.h>
#include "kstfile.h"
#include "kstrvector.h"

void Usage() {
  fprintf(stderr, "usage: d2asc filename [-f <first frame>]\n");
  fprintf(stderr, "             [-n <numframes>] [-s skip [-a]] \n");
  fprintf(stderr, "             [-x] col1 [[-x] col2 ... [-x] coln]\n");
  fprintf(stderr, "   -x specifies that the field should be printed in hex\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  KstFilePtr file;
  int i;

  char field_list[40][120], filename[180];
  bool do_hex[40];
  int n_field=0;
  int start_frame=0, n_frames=2000000;
  bool do_ave = false, do_skip = false;
  int n_skip = 0;
  int NS=0, i_S;

  if (argc < 3 || argv[1][0] == '-')
    Usage();

  for (i = 0; i < 40; i++)
    do_hex[i] = false;

  strcpy(filename, argv[1]);
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
        strcpy(field_list[n_field], argv[i]);
        do_hex[n_field] = true;
        n_field++;
      } else {
        Usage();
      }
    } else {
      strcpy(field_list[n_field], argv[i]);
      n_field++;
    }
  }

  if (!do_skip) do_ave = false;

  file = new KstFile(filename);
  if (file->numFrames() < 1) {
    fprintf(stderr, "d2asc error: file %s has no data\n", filename);
    exit(0);
  }
  /** make vectors and fill the list **/
  QPtrList<KstRVector> vlist;
  vlist.setAutoDelete(true);

  for (i=0; i<n_field; i++) {

    if (!file->isValidField(field_list[i])) {
      fprintf(stderr, "d2asc error: field %s in file %s is not valid\n",
              field_list[i], filename);
      exit(0);
    }
    vlist.append(new KstRVector(file, field_list[i], "tag", start_frame, n_frames, n_skip, n_skip>0, do_ave));
  }

  /* find NS */
  for (i = 0; i < n_field; i++) {

    while (vlist.at(i)->update(-1) != KstObject::NO_CHANGE)
      ; // read vector

    if (vlist.at(i)->sampleCount() > NS)
      NS = vlist.at(i)->sampleCount();
  }

  for (i_S = 0; i_S < NS; i_S++) {
    for (i = 0; i < n_field; i++) {
      if (do_hex[i]) {
        printf("%4x ",  (int)vlist.at(i)->interpolate(i_S, NS));
      } else {
        printf("%.12g ", vlist.at(i)->interpolate(i_S, NS));
      }
    }
    printf("\n");
  }
}
