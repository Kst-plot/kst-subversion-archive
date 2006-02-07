/***************************************************************************
                          main.cpp  -  description
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
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kimageio.h>
#include <klocale.h>
#include <stdio.h>
#include <iostream>

#include "kst.h"
#include "plugincollection.h"
#include "kstvcurve.h"
#include "kstpsdcurve.h"
#include "kstequationcurve.h"
#include "ksthistogram.h"
#include "kstplotlist.h"
#include "kstcolorsequence.h"
#include "kstdatacollection.h"

static const char description[] =
I18N_NOOP("Kst: a data viewing program.");


static KCmdLineOptions options[] = {
  {"F <dataFile>", I18N_NOOP("Specify data file: used to override a kst file default"), "|"},
  { "y <Y>",  I18N_NOOP("Field for Y axis (multiple allowed)"), 0 },
  { "e <E>",  I18N_NOOP("Field for Y errors (multiple allowed)"), 0 },
  { "x <X>",  I18N_NOOP("Field for X axis"),                 "INDEX"},
  { "p <Y>",  I18N_NOOP("Field for power spectrum (multiple allowed)"), 0},
  { "h <Y>",  I18N_NOOP("Field for histogram (multiple allowed)"), 0},
  { "r <f>",  I18N_NOOP("Sample rate for power spectrum"),      "60.0"},
  { "ru <U>", I18N_NOOP("Units for psd sample rate"),           "Hz"},
  { "yu <U>", I18N_NOOP("Units for y vectors"),                 "V"},
  { "l <P>",  I18N_NOOP("Length of FFTs is 2^P"),               "10"},
  { "f <F0>", I18N_NOOP("First frame to read"),                 "-2"},
  { "n <NS>", I18N_NOOP("Number of frames to read"),            "-2"},
  { "s <NS>", I18N_NOOP("Number of frames to skip each read"),  "-1"},
  { "a",      I18N_NOOP("Apply boxcar filter before skipping frames"),0},
  { "m <NC>", I18N_NOOP("Separate plots arranged in <NC> columns"),0},
  { "d",      I18N_NOOP("Display as points rather than curves"),0},
  { "print <file>",  I18N_NOOP("Print to file and exit"),"<none>"},
  { "png <file>",  I18N_NOOP("Save as a png file and exit"),"<none>"},
  { "+[Files]", I18N_NOOP("Data files (if -y given) or *.kst file"), 0},
  KCmdLineLastOption
};

struct InType {
  int skip;
  bool doskip;
  bool doave;
  int n;
  int f;
  double rate;
  int len;
  bool has_points;
  bool sep_plots;
  int n_plots;
  int n_cols;
  int n_rows;
  QString VUnits;
  QString RUnits;
};

/****************************************************************/
/*                                                              */
/*        check command line options for simple usage errors    */
/*                                                              */
/****************************************************************/
void CheckForCMDErrors(KCmdLineArgs *args) {
  bool nOK;

  if (args->getOption("n").toInt(&nOK) == 0) {
    std::cerr << I18N_NOOP("error: exiting because '-n 0' requests vectors with no data in them\n");
    exit(0);
  }
  if (args->getOption("n").toInt(&nOK)>0) {
    if (args->getOption("n").toInt(&nOK) <
        2*args->getOption("s").toInt(&nOK)) {
      std::cerr << "error: requested data skipping would leave vector with fewer than two points\n";
      exit(0);
    }
  }
}


/****************************************************************/
/*                                                              */
/*        fill the in struct from the command line args         */
/*                                                              */
/****************************************************************/
void SetCMDOptions(KCmdLineArgs *args, struct InType &in,
                   QCStringList &ycolList,QCStringList &psdList,
                   QCStringList &hsList) {
  bool nOK;
  in.n = args->getOption("n").toInt(&nOK);
  in.f = args->getOption("f").toInt(&nOK);
  in.rate = args->getOption("r").toDouble(&nOK);
  in.len = args->getOption("l").toInt(&nOK);
  if (in.n<-1) in.n = -1;
  if (in.f<-1) in.f = -1;
  if (in.rate<=0) in.rate = 1.0;
  if (in.len<3) in.len = 3;
  if (in.len>23) in.len = 23;

  in.has_points = args->isSet("d");
  in.n_cols = args->getOption("m").toInt(&nOK);
  if (in.n_cols>0) {
    in.sep_plots = true;
    in.n_plots = args->count() * (ycolList.count()+psdList.count()+
                                  hsList.count());
  } else {
    in.n_cols = 1;
    in.n_plots = 1;
    in.sep_plots = false;
  }
  in.n_rows = (in.n_plots-1)/in.n_cols + 1;

  /* set skip options */
  in.skip = args->getOption("s").toInt(&nOK);
  if (in.skip<1) {
    in.skip = 0;
    in.doskip = false;
    in.doave = false;
  } else {
    in.doskip = true;
    in.doave = args->isSet("a");
  }

  /* set Units for PSDs */
  in.VUnits = args->getOption("yu");
  in.RUnits = args->getOption("ru");
}

/****************************************************************/
/*                                                              */
/*        create the plots                                      */
/*                                                              */
/****************************************************************/
void CreatePlots(struct InType &in) {
  KstPlot *plot;
  int i_plot;
  int size_boost;

  for (i_plot=0; i_plot < in.n_plots; i_plot++) {
    plot = new KstPlot(QString("P") + QString::number(i_plot),
                       1.0/double(in.n_cols), 1.0/double(in.n_rows),
                       double(i_plot%in.n_cols)/double(in.n_cols),
                       double(i_plot/in.n_cols)/double(in.n_rows));

    size_boost = (in.n_rows + in.n_cols)/2-1;
    size_boost = size_boost*12/2;
    plot->initFonts(QFont::QFont(), size_boost);
    KST::plotList.append(plot);
  }
}

/****************************************************************/
/*                                                              */
/*        create vectors, including name and error cheking      */
/*                                                              */
/****************************************************************/
KstRVectorPtr CreateVector(QString field, KstFilePtr file, struct InType &in) {
  KstRVectorPtr vector;
  static int i_v=0;

  vector = new KstRVector(file, field,
                          QString("V") + QString::number(1+i_v++)
                          + "-" + field,
                          in.f, in.n, in.skip, in.doskip, in.doave);
  if (!vector->isValid()) {
    kdError() << I18N_NOOP("Error: Invalid field: ") << vector->getField()
         << endl << I18N_NOOP("File: ") << vector->getFilename() << endl;
    exit(0); // fixme: this can cause crashes
  }

  return vector;
}

/****************************************************************/
/*                                                              */
/*        main for kst.  mostly command line handling           */
/*                                                              */
/****************************************************************/
int main(int argc, char *argv[]) {
  int i_file, i_v, i_curve;
  int i_plot;

  KAboutData aboutData( "kst", I18N_NOOP("Kst"),
                        "0.93-planckio-branch", description, KAboutData::License_GPL,
                        I18N_NOOP("(c) 2000-2003 Barth Netterfield"),
                        0,
                        "http://extragear.kde.org/apps/kst.php");
  aboutData.addAuthor("Barth Netterfield",
                      I18N_NOOP("Original author and maintainer."),
                      "netterfield@astro.utoronto.ca",
                      "http://omega.astro.utoronto.ca/");
  aboutData.addAuthor("Staikos Computing Services Inc.",
                      I18N_NOOP("Developed for the University of Toronto."),
                      "info@staikos.net",
                      "http://www.staikos.net/");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;
  KImageIO::registerFormats();

  if (app.isRestored()) {
    RESTORE(KstApp)
  } else {
    KstApp *kst = new KstApp;

    struct InType in;
    QColor color;
    QCStringList ycolList;
    QCStringList psdList;
    QCStringList hsList;
    QCStringList errorList;
    unsigned int i_ycol;
    QCStringList::Iterator psd;
    QCStringList::Iterator hs;
    bool nOK;

    /* temp variables: these all get stuck into list objects */
    KstFilePtr file;
    KstRVectorPtr xvector;
    KstRVectorPtr yvector;
    KstRVectorPtr evector;
    KstVCurve *curve;
    KstPSDCurve *psdcurve;
    KstHistogramPtr hscurve;
    KstPlot *plot;

    /* Parse command line args */
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    CheckForCMDErrors(args);

    // Initialise the plugin loader and collection.
    PluginCollection::self();

    /* get Y axis collums */
    ycolList = args->getOptionList("y");
    psdList = args->getOptionList("p");
    hsList = args->getOptionList("h");
    errorList = args->getOptionList("e");
    // y axis or PSD specified, so the files are data files, not kst files.
    if (ycolList.count() + psdList.count() + hsList.count() > 0) {

      SetCMDOptions(args, in, ycolList, psdList, hsList);

      CreatePlots(in);

      i_plot = 0;
      plot = KST::plotList.at(i_plot);

      /* Make the requested curves for each data file */
      for (i_curve = i_v = 0, i_file = 0; i_file < args->count(); i_file++) {
        /* Make the file */
        file = new KstFile(args->arg(i_file));

        if (file->numFrames()<1) { // No data in file
          std::cerr << I18N_NOOP("Error: No data in file: ")
	       << args->arg(i_file) << "\n";
          delete kst;
          exit(0);
        }
        KST::fileList.append(file);

        if (!ycolList.isEmpty()) { // if there are some xy plots
          /* make the x axis vector */
          xvector = CreateVector(args->getOption("x"), file, in);

          /* make the y axis vectors */
          for (i_ycol = 0; i_ycol < ycolList.count(); ++i_ycol ) {
            /*for (ycol = ycolList.begin(); ycol != ycolList.end(); ++ycol ) */
            yvector = CreateVector(*(ycolList.at(i_ycol)), file, in);

            /* make the curves */
            color = KstColorSequence::next();
            curve = new KstVCurve(QString("C") + QString::number(1+i_curve++)
                                  + "-" + yvector->getField(),
                                  KstVectorPtr(xvector), KstVectorPtr(yvector),
                                  0L, 0L, color);
            if (in.has_points) {
              curve->setHasPoints(true);
              curve->setHasLines(false);
            }

            if (i_ycol<errorList.count()) {
              evector = CreateVector(*(errorList.at(i_ycol)), file, in);
              curve->setYError(KstVectorPtr(evector));
            }
            KST::dataObjectList.append(curve);
            plot->addCurve(curve);

            if (in.sep_plots) {
              i_plot++;
              if (i_plot < in.n_plots)
                plot = KST::plotList.at(i_plot);
            } // end (if they are separate plots)
          } // next y col
        } // end (if there are some xy plots)
        if (psdList.count() > 0) { // if there are some psd plots
          KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
          for(psd = psdList.begin(); psd != psdList.end(); ++psd ) {
            KstRVectorList::Iterator it;
            for (it = rvl.begin(); it != rvl.end(); ++it) {
              if ((*it)->getField() == (*psd).data()) {
                break;
              }
            }
            if (it != rvl.end()) {
              yvector = *it;
            } else {
              yvector = CreateVector(*psd, file, in);
            } // end (if the vector hasn't yet been created)

            color = KstColorSequence::next();

            psdcurve = new KstPSDCurve(QString("P") +
                                       QString::number(1+i_curve++)
                                       + "-" + yvector->getField(),
                                       KstVectorPtr(yvector), in.rate, in.len,
                                       in.VUnits,in.RUnits,
                                       color);
            if (in.has_points) {
              psdcurve->setHasPoints(true);
              psdcurve->setHasLines(false);
            }
            KST::dataObjectList.append(psdcurve);
            plot->addCurve(psdcurve);

            if (in.sep_plots) {
              i_plot++;
              if (i_plot <in.n_plots) plot = KST::plotList.at(i_plot);
            }
          } // next psd
        } // end (if there are some psds)
        if (hsList.count()>0) { // if there are some histograms
          double max, min, m;
          int N;

          KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
          for (hs = hsList.begin(); hs != hsList.end(); ++hs ) {
            KstRVectorList::Iterator it;
            for (it = rvl.begin(); it != rvl.end(); ++it) {
              if ((*it)->getField() == (*hs).data()) {
                break;
              }
            }
            if (it != rvl.end()) {
              yvector = *it;
            } else {
              yvector = CreateVector(*hs, file, in);
            } // end (if the vector hasn't yet been created)

            color = KstColorSequence::next();
            max = yvector->max();
            min = yvector->min();
            N = yvector->sampleCount();

            if (max<min) {
              m = max;
              max = min;
              min = m;
            }
            if (max==min) {
              max+=1;
              min -=1;
            }

            N /= 50;
            if (N < 6) {
              N = 6;
            } else if (N > 60) {
              N = 60;
            }

            m = (max -min)/(100.0*double(N));
            max += m;
            min -= m;

            hscurve = new KstHistogram(QString("H") +
                                       QString::number(1+i_curve++)
                                       + "-" + yvector->getField(),
                                       KstVectorPtr(yvector), min, max, N,
                                       KST_HS_NUMBER,
                                       color);
            KST::dataObjectList.append(KstDataObjectPtr(hscurve));
            plot->addCurve(hscurve);

            if (in.sep_plots) {
              i_plot++;
              if (i_plot < in.n_plots)
                plot = KST::plotList.at(i_plot);
            }
          } // next histogram
        } // end (if there are some histograms)
      } // next data file
      for (i_plot = 0; i_plot < in.n_plots; i_plot++) {
        KST::plotList.at(i_plot)->GenerateDefaultLabels();
      }
      KST::plotList.setPlotCols(in.n_cols);

    } else if (args->count() > 0) {
      /* open a kst file */
      /* some of the options can be overridden */
      kst->openDocumentFile(args->arg(0),
			    args->getOption("F"), // override FileName
			    // override number of frames
			    args->getOption("n").toInt(&nOK),
			    // override starting frame
			    args->getOption("f").toInt(&nOK),
			    // override skip
			    args->getOption("s").toInt(&nOK),
			    // add averaging
			    args->isSet("a"));
    } else {
      //kst->openDocumentFile();
    }

    QString printfile;
    printfile = args->getOption("print");
    QString pngfile;
    pngfile = args->getOption("png");
    bool print_and_exit = false;

    if (printfile!="<none>") {
      args->clear();
      kst->forcedUpdate();
      kst->immediatePrintToFile(printfile);
      print_and_exit = true;
    }

    if (pngfile!="<none>") {
      args->clear();
      kst->forcedUpdate();
      kst->immediatePrintToPng(pngfile);
      print_and_exit = true;
    }

    if (print_and_exit) {
      delete kst;
      exit(0);
    } else {
      args->clear();
      app.setMainWidget(kst);
      kst->show();
    }
    // LEAVE THIS HERE - causes crashes otherwise!
    int rc = app.exec();
    delete kst;
    return rc;
  }
  return app.exec();
}
