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

static const char description[] = I18N_NOOP("Kst: a data viewing program.");


static KCmdLineOptions options[] = {
  {"F <dataFile>", I18N_NOOP("Specify data file: used to override a kst file default"), "|"},
  { "y <Y>",  I18N_NOOP("Field for Y axis (multiple allowed)"), 0 },
  { "ye <equation>",  I18N_NOOP("Equation for Y axis (multiple allowed)"), 0 },
  { "e <E>",  I18N_NOOP("Field for Y errors (multiple allowed)"), 0 },
  { "x <X>",  I18N_NOOP("Field or range for X axis"),        "INDEX"},
  { "xe <X>", I18N_NOOP("X vector for equations x0:x1:n"),   "INDEX"},
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
  { "g",      I18N_NOOP("Provide a legend box"),0},
  { "print <file>",  I18N_NOOP("Print to file and exit"),"<none>"},
  { "png <file>",  I18N_NOOP("Save as a png file and exit"),"<none>"},
  { "+[Files]", I18N_NOOP("Data files (if -y given) or *.kst file"), 0},
  KCmdLineLastOption
};

struct InType {
  int skip;
  bool doskip;
  bool doave;
  bool dolegend;
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
                   int n_y) {
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
    if (args->count()>0) {
      in.sep_plots = true;
      in.n_plots = args->count() * (n_y);
    } else if (args->getOptionList("ye").count()>0) {
      in.sep_plots = true;
      in.n_plots = args->getOptionList("ye").count();
    }
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
  in.dolegend = args->isSet("g");
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

/******************************************************************/
/*                                                                */
/*        set X range for equations if -xe is specified           */
/*        start:end:Nsamp                                         */
/*                                                                */
/******************************************************************/
void SetEqXRanges(QString xeqS, double *min, double *max, int *n, bool *ok) {
  *ok = true;
  QStringList fields = QStringList::split( QChar(':'), xeqS );
  if (fields.count()!=3) {
    *ok=false;
    *n=2;
    *min = -1;
    *max = 1;
    return;
  }
  QString f;
  f = *fields.at(0);
  *min = f.toDouble(ok);
  f = *fields.at(1);
  *max = f.toDouble(ok);
  f = *fields.at(2);
  *n = f.toInt(ok);

  if (*n<2) *ok=false;
  if (*min>=*max) *ok = false;
}

/******************************************************************/
/*                                                                */
/*        create vectors, including name and error checking       */
/*        if a vector with the same field and file exists, use it */
/*                                                                */
/******************************************************************/
KstRVector *GetOrCreateVector(QString field, KstDataSourcePtr file, struct InType &in) {
  int i_v = 0, n_v;
  KstRVector *V;
  KstRVectorList vectorList =
    kstObjectSubList<KstVector,KstRVector>(KST::vectorList);

  n_v = vectorList.count();

  for (i_v=0; i_v<n_v; i_v++) {
    V = *vectorList.at(i_v);
    if (V->getField() == field) {
      if (V->filename() == file->fileName()) {
	return (V);
      }
    }
  }
  V = new KstRVector(file, field,
			  QString("V") + QString::number(1 + i_v++)
                          + "-" + field,
                          in.f, in.n, in.skip, in.doskip, in.doave);
  if (!V->isValid()) {
    kdError() << I18N_NOOP("Error: Invalid field: ") << V->getField()
              << endl << I18N_NOOP("File: ") << V->filename() << endl;
    exit(0); // fixme: this can cause crashes
  }

  return V;
}

/****************************************************************/
/*                                                              */
/*        Process Equation: replace fields with vector tags     */
/*        Create vectors that don't exist.                      */
/*                                                              */
/****************************************************************/
bool NoVectorEq(QString eq) {
  if (eq.find("[")>=0)
    return false;
  else
    return true;
}

void ProcessEq(QString &eq, KstDataSourcePtr file,
               struct InType &in, bool *ok) {
  QString field;
  int i0=0, i1=0;
  KstVector *v;
  *ok = true;

  while ((i0 = eq.find("[", i0))>=0) {
    i1 = eq.find("]", i0);
    if (i1>0) {
      field = eq.mid(i0+1, i1-i0-1);
      v = GetOrCreateVector(field, file, in);
      eq.replace(i0,i1-i0+1, "["+v->tagName()+"]");
    }
    i0++;
  }
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
                        "0.96-devel", description, KAboutData::License_GPL,
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
    QCStringList yEqList;
    QCStringList psdList;
    QCStringList hsList;
    QCStringList errorList;
    unsigned int i_ycol;
    QCStringList::Iterator psd;
    QCStringList::Iterator hs;
    QCStringList::Iterator eq_i;
    bool nOK;

    /* temp variables: these all get stuck into list objects */
    KstDataSourcePtr file;
    KstRVector *xvector=NULL;
    KstRVector *yvector;
    KstRVector *evector;
    KstVCurve *curve;
    KstPSDCurve *psdcurve;
    KstEquationCurve *eqcurve;
    KstHistogram *hscurve;
    KstPlot *plot;
    int n_y, n_eq=0;

    /* Parse command line args */
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    CheckForCMDErrors(args);

    // Initialise the plugin loader and collection.
    PluginCollection::self();

    /* get Y axis collums */
    ycolList = args->getOptionList("y");
    yEqList = args->getOptionList("ye");
    psdList = args->getOptionList("p");
    hsList = args->getOptionList("h");
    errorList = args->getOptionList("e");

    // y axis or PSD specified, so the files are data files, not kst files.
    n_y = ycolList.count() + psdList.count() + hsList.count()
      + yEqList.count();
    if (n_y > 0) {

      SetCMDOptions(args, in, n_y);

      CreatePlots(in);

      i_plot = 0;
      plot = KST::plotList.at(i_plot);

      /* make stand alone equations if there are no files */
      if (args->count()<1) {
        if (!yEqList.isEmpty()) {
	  QString eqS;
          double max, min;
          int n;
          bool xeq;
          SetEqXRanges(args->getOption("xe"), &min, &max, &n, &xeq);
          if (xeq) {
            for (eq_i = yEqList.begin(); eq_i != yEqList.end(); ++eq_i) {
              eqS = *eq_i;
              if (NoVectorEq(eqS)) {
                eqcurve =
                  new KstEquationCurve(QString("E")+QString::number(n_eq+1)+
                                       "-" + eqS,
                                       eqS,
                                       min,max,n, KstColorSequence::next());

                KST::dataObjectList.lock().writeLock();
                KST::dataObjectList.append(eqcurve);
                KST::dataObjectList.lock().writeUnlock();
                plot->addCurve(eqcurve);

                if (in.sep_plots) {
                  i_plot++;
                  if (i_plot <in.n_plots) plot = KST::plotList.at(i_plot);
                }
              }
            }
          }
        }
      }

      /* Make the requested curves for each data file */
      for (i_curve = i_v = 0, i_file = 0; i_file < args->count(); i_file++) {
        /* Make the file */
        file = KstDataSource::loadSource(args->arg(i_file));

        if (!file) {
          std::cerr << I18N_NOOP("Error: No data in file: ")
	       << args->arg(i_file) << "\n";
          delete kst;
          exit(0);
        }

	if (!file->isValid() || file->frameCount() < 1) {
          std::cerr << I18N_NOOP("No data in file: ")
	       << args->arg(i_file)
	       << ".  Trying to continue..."
	       << "\n";
          // The file might get data later!
	}

        KST::dataObjectList.lock().writeLock();
        KST::dataSourceList.append(file);
        KST::dataObjectList.lock().writeUnlock();

        if (!ycolList.isEmpty()) { // if there are some xy plots
          /* make the x axis vector */
          xvector = GetOrCreateVector(args->getOption("x"), file, in);

          /* make the y axis vectors */
          for (i_ycol = 0; i_ycol < ycolList.count(); ++i_ycol ) {
            yvector = GetOrCreateVector(*(ycolList.at(i_ycol)), file, in);

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
              evector = GetOrCreateVector(*(errorList.at(i_ycol)), file, in);
              curve->setYError(KstVectorPtr(evector));
            }

            KST::dataObjectList.lock().writeLock();
            KST::dataObjectList.append(curve);
            KST::dataObjectList.lock().writeUnlock();
            plot->addCurve(curve);

            if (in.sep_plots) {
              i_plot++;
              if (i_plot < in.n_plots)
                plot = KST::plotList.at(i_plot);
            } // end (if they are separate plots)
          } // next y col
        } // end (if there are some xy plots)
	if (!yEqList.isEmpty()) {
	  QString eqS;
          double max, min;
          int n;
          bool xeq, eq_ok;

          SetEqXRanges(args->getOption("xe"), &min, &max, &n, &xeq);
	  for (eq_i = yEqList.begin(); eq_i != yEqList.end(); ++eq_i) {
	    eqS = *eq_i;
	    ProcessEq(eqS, file, in, &eq_ok);
            if (xeq) {
              eqcurve =
                new KstEquationCurve(QString("E")+QString::number(n_eq+1)+
                                     "-" + eqS,
                                     eqS,
                                     min,max,n, KstColorSequence::next());
            } else {
              if (xvector==NULL)
                xvector = GetOrCreateVector(args->getOption("x"), file, in);

              eqcurve =
                new KstEquationCurve(QString("E")+QString::number(n_eq+1)+eqS,
                                     eqS,
                                     KstVectorPtr(xvector),
                                     true, KstColorSequence::next());
            }
            KST::dataObjectList.lock().writeLock();
            KST::dataObjectList.append(eqcurve);
            KST::dataObjectList.lock().writeUnlock();
            plot->addCurve(eqcurve);

            if (in.sep_plots) {
              i_plot++;
              if (i_plot <in.n_plots) plot = KST::plotList.at(i_plot);
            }
	  }
	}
        if (psdList.count() > 0) { // if there are some psd plots
          KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
          for(psd = psdList.begin(); psd != psdList.end(); ++psd ) {

	    yvector = GetOrCreateVector(*psd, file, in);

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
            KST::dataObjectList.lock().writeLock();
            KST::dataObjectList.append(psdcurve);
            KST::dataObjectList.lock().writeUnlock();
            plot->addCurve(psdcurve);

            if (in.sep_plots) {
              i_plot++;
              if (i_plot <in.n_plots) plot = KST::plotList.at(i_plot);
            }
          } // next psd
        } // end (if there are some psds)
        if (hsList.count()>0) { // if there are some histograms
          double max, min;
          int N;

          KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
          for (hs = hsList.begin(); hs != hsList.end(); ++hs ) {
	    yvector = GetOrCreateVector(*hs, file, in);

            color = KstColorSequence::next();

            KstHistogram::AutoBin(KstVectorPtr(yvector), &N, &max, &min);

            hscurve = new KstHistogram(QString("H") +
                                       QString::number(1+i_curve++)
                                       + "-" + yvector->getField(),
                                       KstVectorPtr(yvector), min, max, N,
                                       KST_HS_NUMBER,
                                       color);
            KST::dataObjectList.lock().writeLock();
            KST::dataObjectList.append(KstDataObjectPtr(hscurve));
            KST::dataObjectList.lock().writeUnlock();
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
        plot = KST::plotList.at(i_plot);
        plot->GenerateDefaultLabels();
        if (in.dolegend) {
          plot->Legend->setShow(true);
          plot->Legend->setFront(true);
        }
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
      kst->forceUpdate();
      kst->immediatePrintToFile(printfile);
      print_and_exit = true;
    }

    if (pngfile!="<none>") {
      args->clear();
      kst->forceUpdate();
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
// vim: ts=2 sw=2 et
