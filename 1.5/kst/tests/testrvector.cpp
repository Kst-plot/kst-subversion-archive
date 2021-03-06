/*
 *  Copyright 2005, The University of Toronto
 *  Licensed under GPL.
 */

// HACK to get at methods we shouldn't be getting at
#define protected public
#include "kstdatasource.h"
#undef protected

#include "ksttestcase.h"
#include <kconfig.h>
#include <kstdataobjectcollection.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <qdir.h>
#include <qfile.h>

#include "kstrvector.h"

static void exitHelper() {
  KST::dataSourceList.clear();
  KST::vectorList.clear();
  KST::scalarList.clear();
  KST::dataObjectList.clear();
}


int rc = KstTestSuccess;

#define doTest(x) testAssert(x, QString("Line %1").arg(__LINE__))
#define doTestD(x, y) testAssert(x, QString("%1: %2").arg(__LINE__).arg(y))
#define doTestV(name, expect, actual) testAssert((expect == actual), QString("Line:%1 %2-->Expect:%3 | Actual:%4").arg(__LINE__).arg(name).arg(expect).arg(actual))

void testAssert(bool result, const QString& text = "Unknown") {
  if (!result) {
    KstTestFailed();
    printf("Test [%s] failed.\n", text.latin1());
  }
}


//////////////////////////////////////////////////////////////////////////////
void testAscii() {
  QFile *qf;

  {
    KTempFile tf(locateLocal("tmp", "kst-test-ascii"), "txt");
    qf = tf.file();
    QTextStream ts(qf);
    ts << ";" << endl;
    ts << " ;" << endl;
    ts << "#;" << endl;
    ts << "c comment comment" << endl;
    ts << "\t!\t!\t!\t!" << endl;
    ts << "2.0" << endl;
    ts << "1" << endl;
    ts << ".2" << endl;

    tf.close();

    KstDataSourcePtr dsp = KstDataSource::loadSource(tf.name());

    doTest(dsp);
    doTest(dsp->isValid());
    doTest(dsp->isValidField("INDEX"));
    doTest(dsp->isValidField("1"));
    doTest(!dsp->isValidField("0"));
    doTest(!dsp->isValidField("2"));
    doTest(dsp->samplesPerFrame(QString::null) == 1);
    doTest(dsp->samplesPerFrame("INDEX") == 1);
    doTest(dsp->samplesPerFrame("1") == 1);
    doTest(dsp->frameCount(QString::null) == 3);
    doTest(dsp->frameCount("1") == 3);
    doTest(dsp->frameCount("INDEX") == 3);
    doTest(dsp->fileName() == tf.name());
    doTest(dsp->fieldList().count() == 2);
    doTest(dsp->fieldListIsComplete());
    doTest(!dsp->isEmpty());

    KstRVectorPtr rvp = new KstRVector(dsp, "1", KstObjectTag("RVTestAscii1"), 0, -1, 0, false, false);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 3);
    doTest(rvp->value()[0] == 2.0);
    doTest(rvp->value()[1] == 1.0);
    doTest(rvp->value()[2] == 0.2);
    rvp = new KstRVector(dsp, "INDEX", KstObjectTag("RVTestAscii2"), 0, -1, 0, false, false);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 3);
    doTest(rvp->value()[0] == 0.0);
    doTest(rvp->value()[1] == 1.0);
    doTest(rvp->value()[2] == 2.0);

    QFile::remove(tf.name());
  }

  {
    KTempFile tf(locateLocal("tmp", "kst-test-ascii"), "txt");
    qf = tf.file();
    QTextStream ts(qf);
    ts << "2e-1 \t .1415" << endl;
    ts << "nan -.4e-2" << endl;
    ts << "inf\t1" << endl;
    ts << "0.000000000000000000000000000000000000000000000000 0" << endl;

    tf.close();

    KstDataSourcePtr dsp = KstDataSource::loadSource(tf.name());

    doTest(dsp);
    doTest(dsp->isValid());
    doTest(dsp->isValidField("INDEX"));
    doTest(dsp->isValidField("1"));
    doTest(!dsp->isValidField("0"));
    doTest(dsp->isValidField("2"));
    doTest(!dsp->isValidField("3"));
    doTest(dsp->samplesPerFrame(QString::null) == 1);
    doTest(dsp->samplesPerFrame("INDEX") == 1);
    doTest(dsp->samplesPerFrame("1") == 1);
    doTest(dsp->samplesPerFrame("2") == 1);
    doTest(dsp->frameCount(QString::null) == 4);
    doTest(dsp->frameCount("1") == 4);
    doTest(dsp->frameCount("2") == 4);
    doTest(dsp->frameCount("INDEX") == 4);
    doTest(dsp->fieldList().count() == 3);
    doTest(!dsp->isEmpty());

    KstRVectorPtr rvp = new KstRVector(dsp, "1", KstObjectTag("RVTestAscii1"), 0, -1, 0, false, false);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 4);
    doTest(rvp->value()[0] == 0.2);
    doTest(rvp->value()[1] != rvp->value()[1]);
    doTest(rvp->value()[2] == INF);
    doTest(rvp->value()[3] == 0);
    rvp = new KstRVector(dsp, "2", KstObjectTag("RVTestAscii2"), 0, -1, 0, false, false);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 4);
    doTest(rvp->value()[0] == 0.1415);
    doTest(rvp->value()[1] == -0.004);
    doTest(rvp->value()[2] == 1.0);
    doTest(rvp->value()[3] == 0.0);

    QFile::remove(tf.name());
  }

  {
    KTempFile tf(locateLocal("tmp", "kst-test-ascii"), "txt");
    qf = tf.file();
    QTextStream ts(qf);
    ts << "2 4" << endl;

    tf.close();

    KstDataSourcePtr dsp = KstDataSource::loadSource(tf.name());

    doTest(dsp);
    doTest(dsp->isValid());
    doTest(dsp->isValidField("INDEX"));
    doTest(dsp->isValidField("1"));
    doTest(!dsp->isValidField("0"));
    doTest(dsp->isValidField("2"));
    doTest(!dsp->isValidField("3"));
    doTest(dsp->samplesPerFrame(QString::null) == 1);
    doTest(dsp->samplesPerFrame("INDEX") == 1);
    doTest(dsp->samplesPerFrame("1") == 1);
    doTest(dsp->samplesPerFrame("2") == 1);
    doTest(dsp->frameCount(QString::null) == 1);
    doTest(dsp->frameCount("1") == 1);
    doTest(dsp->frameCount("2") == 1);
    doTest(dsp->frameCount("INDEX") == 1);
    doTest(dsp->fieldList().count() == 3);
    doTest(!dsp->isEmpty());

    KstRVectorPtr rvp = new KstRVector(dsp, "1", KstObjectTag("RVTestAscii1"), 0, -1, 0, false, false);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 1); // Are we allowed to have vectors of 1?
    doTest(rvp->value()[0] == 2);
    rvp = new KstRVector(dsp, "2", KstObjectTag("RVTestAscii2"), 0, -1, 0, false, false);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 1);
    doTest(rvp->value()[0] == 4);

    QFile::remove(tf.name());
  }

  {
    KTempFile tf(locateLocal("tmp", "kst-test-ascii"), "txt");
    qf = tf.file();
    QTextStream ts(qf);
    ts << ";" << endl;

    tf.close();

    KstDataSourcePtr dsp = KstDataSource::loadSource(tf.name());

    doTest(dsp);
    QFile::remove(tf.name());
  }

  {
    KTempFile tf(locateLocal("tmp", "kst-test-ascii"), "txt");
    qf = tf.file();
    QTextStream ts(qf);
    for (int i = 0; i < 39000; ++i) {
      ts << i << " " <<  i + 100 << " " << i + 1000 << endl;
    }

    tf.close();

    KstDataSourcePtr dsp = KstDataSource::loadSource(tf.name());
    dsp->update(0);

    doTest(dsp);
    doTest(dsp->isValid());
    doTest(dsp->frameCount(QString::null) == 39000);
    doTest(dsp->frameCount("1") == 39000);
    doTest(dsp->frameCount("2") == 39000);
    doTest(dsp->frameCount("3") == 39000);
    doTest(dsp->frameCount("INDEX") == 39000);
    doTest(dsp->fieldList().count() == 4);
    doTest(!dsp->isEmpty());

    KstRVectorPtr rvp = new KstRVector(dsp, "1", KstObjectTag("RVTestAscii1"), 0, -1, 0, false, false);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 39000);
    rvp = new KstRVector(dsp, "2", KstObjectTag("RVTestAscii2"), 0, -1, 10, true, false);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 3900);
    doTest(rvp->value()[0] == 100);
    doTest(rvp->value()[1] == 110);
    doTest(rvp->value()[2] == 120);
    doTest(rvp->value()[3898] == 39080);

    rvp = new KstRVector(dsp, "3", KstObjectTag("RVTestAscii2"), 0, -1, 10, true, true);
    rvp->update(0);
    doTest(rvp->isValid());
    doTest(rvp->length() == 3900);
    doTest(rvp->value()[0] == 1004.5);
    doTest(rvp->value()[1] == 1014.5);
    doTest(rvp->value()[2] == 1024.5);
    doTest(rvp->value()[3898] == 39984.5);

    QFile::remove(tf.name());

    rvp->reload();
    doTest(!rvp->isValid());
  }
}


//////////////////////////////////////////////////////////////////////////////
void testCDF() {
}


//////////////////////////////////////////////////////////////////////////////
void testDirfile(const char *srcfile) {

  Q_UNUSED(srcfile);
  //WARNING! These tests assume that the dirfile was generated with dirfile_maker

  //FIXME handle dirfile passed from commandline...

  {
    QString fifteen = QDir::currentDirPath() + QDir::separator() + QString("dirfile_testcase") +
                      QDir::separator() + QString("15count");
    if (!QFile::exists(fifteen + QDir::separator() + "format")) {
      return;
    }
    printf("Opening dirfile = %s for test.\n", fifteen.latin1());

    KstDataSourcePtr dsp = KstDataSource::loadSource(fifteen);
    dsp->update(0);

    doTest(dsp);
    doTest(dsp->isValid());
    doTest(dsp->isValidField("INDEX"));
    doTest(dsp->frameCount("INDEX") == 15);
    doTest(dsp->isValidField("cos"));
    doTest(dsp->isValidField("fcount"));
    doTest(dsp->isValidField("scount"));
    doTest(dsp->isValidField("sine"));
    doTest(dsp->isValidField("ssine"));
    doTest(!dsp->isValidField("foo"));

    //TODO test samples per frame?

    doTest(dsp->fileName() == fifteen);
    doTest(dsp->fieldList().count() == 6);
    doTest(dsp->fieldListIsComplete());

    doTest(!dsp->isEmpty());

  {
    //Skip FIVE frames...
    KstRVectorPtr rvp = new KstRVector(dsp, "INDEX", KstObjectTag("RVTestDirfile"), 0, -1, 5, true, false);
    rvp->update(0);

    // we have requested 1 value for every five frames, starting at frame 0. 
    // As we have 15 frames in the data we should effectively read 15 frames 
    // and return 3 values...
    doTestV("length", 3, rvp->length());
    doTestV("item #0", 0, rvp->value(0));
    doTestV("item #1", 5, rvp->value(1));
    doTestV("item #2", 10, rvp->value(2));

    doTestV("numFrames", 15, rvp->numFrames());
    doTestV("startFrame", 0, rvp->startFrame());

    doTestV("reqNumFrames", -1, rvp->reqNumFrames());
    doTestV("reqStartFrame", 0, rvp->reqStartFrame());

    doTestV("readToEOF", true, rvp->readToEOF());
    doTestV("countFromEOF", false, rvp->countFromEOF());
    doTestV("doSkip", true, rvp->doSkip());
    doTestV("skip", 5, rvp->skip());
    doTestV("doAve", false, rvp->doAve());
  }
  {
    //Skip FIVE frames...
    KstRVectorPtr rvp = new KstRVector(dsp, "INDEX", KstObjectTag("RVTestDirfile"), 3, -1, 5, true, false);
    rvp->update(0);

    // we have requested 1 value for every five frames, starting at frame 3.
    // As we read data only at skip boundaries we will actually read starting
    // at frame 5. As we have 10 frames left in the data we should effectively 
    // read 10 frames and return 2 values...
    doTestV("length", 2, rvp->length());
    doTestV("item #0", 5, rvp->value(0));
    doTestV("item #1", 10, rvp->value(1));

    doTestV("numFrames", 10, rvp->numFrames());
    doTestV("startFrame", 5, rvp->startFrame());

    doTestV("reqNumFrames", -1, rvp->reqNumFrames());
    doTestV("reqStartFrame", 3, rvp->reqStartFrame());

    doTestV("readToEOF", true, rvp->readToEOF());
    doTestV("countFromEOF", false, rvp->countFromEOF());
    doTestV("doSkip", true, rvp->doSkip());
    doTestV("skip", 5, rvp->skip());
    doTestV("doAve", false, rvp->doAve());
  }
  {
    //Skip FIVE frames...
    KstRVectorPtr rvp = new KstRVector(dsp, "INDEX", KstObjectTag("RVTestDirfile"), 0, 11, 5, true, false);
    rvp->update(0);

    // we have requested 1 value for every five frames, starting at frame 0
    // and reading to frame 11. As we read data only at skip boundaries we will 
    // actually read starting at frame 0 and ending at frame 9. So we have effectively
    // read 10 frames and return 2 values...
    doTestV("length", 2, rvp->length());
    doTestV("item #0", 0, rvp->value(0));
    doTestV("item #1", 5, rvp->value(1));

    doTestV("numFrames", 10, rvp->numFrames());
    doTestV("startFrame", 0, rvp->startFrame());

    doTestV("reqNumFrames", 11, rvp->reqNumFrames());
    doTestV("reqStartFrame", 0, rvp->reqStartFrame());

    doTestV("readToEOF", false, rvp->readToEOF());
    doTestV("countFromEOF", false, rvp->countFromEOF());
    doTestV("doSkip", true, rvp->doSkip());
    doTestV("skip", 5, rvp->skip());
    doTestV("doAve", false, rvp->doAve());
  }
  {
    //Skip FIVE frames and countFromEOF()...
    KstRVectorPtr rvp = new KstRVector(dsp, "INDEX", KstObjectTag("RVTestDirfile"), -1, 10, 5, true, false);
    rvp->update(0);

    // we have requested 1 value for every five frames, reading 10 frames
    // from the end of the file. As we read data only at skip boundaries we will 
    // actually read starting at frame 5 and ending at frame 14. So we have effectively
    // read 10 frames and return 2 values...
    doTestV("length", 2, rvp->length());
    doTestV("item #0", 5, rvp->value(0));
    doTestV("item #1", 10, rvp->value(1));

    doTestV("numFrames", 10, rvp->numFrames());
    doTestV("startFrame", 5, rvp->startFrame());

    doTestV("reqNumFrames", 10, rvp->reqNumFrames());
    doTestV("reqStartFrame", -1, rvp->reqStartFrame());

    doTestV("readToEOF", false, rvp->readToEOF());
    doTestV("countFromEOF", true, rvp->countFromEOF());
    doTestV("doSkip", true, rvp->doSkip());
    doTestV("skip", 5, rvp->skip());
    doTestV("doAve", false, rvp->doAve());
  }
  }
}


//////////////////////////////////////////////////////////////////////////////
void testFrame() {
}


//////////////////////////////////////////////////////////////////////////////
void testIndirect() {
}


//////////////////////////////////////////////////////////////////////////////
void testLFI() {
}


//////////////////////////////////////////////////////////////////////////////
void testPlanck() {
}


//////////////////////////////////////////////////////////////////////////////
void testStdin() {
}


//////////////////////////////////////////////////////////////////////////////
void doTests(const char *srcfile) {
  QStringList plugins = KstDataSource::pluginList();
  for (QStringList::ConstIterator i = plugins.begin(); i != plugins.end(); ++i) {
    printf("Found data source plugin: %s\n", (*i).latin1());
  }

  if (plugins.contains("ASCII File Reader")) {
    testAscii();
  } else {
    printf("Not running tests for ASCII - couldn't find plugin.\n");
  }

  if (plugins.contains("CDF File Reader")) {
    testCDF();
  } else {
    printf("Not running tests for CDF - couldn't find plugin.\n");
  }

  if (plugins.contains("DirFile Reader")) {
    testDirfile(srcfile);
  } else {
    printf("Not running tests for dirfile - couldn't find plugin.\n");
  }

  if (plugins.contains("Frame Reader")) {
    testFrame();
  } else {
    printf("Not running tests for frame files - couldn't find plugin.\n");
  }

  if (plugins.contains("Indirect File Reader")) {
    testIndirect();
  } else {
    printf("Not running tests for Indirect - couldn't find plugin.\n");
  }

  if (plugins.contains("LFIIO Reader")) {
    testLFI();
  } else {
    printf("Not running tests for LFI - couldn't find plugin.\n");
  }

  if (plugins.contains("PLANCK Plugin")) {
    testPlanck();
  } else {
    printf("Not running tests for Planck - couldn't find plugin.\n");
  }

  testStdin();

  // Tests for the kstrvector class to complete coverage go here.

}


int main(int argc, char **argv) {
  atexit(exitHelper);

  KApplication app(argc, argv, "testrvector", false, false);

  KConfig *kConfigObject = new KConfig("kstdatarc", false, false);
  KstDataSource::setupOnStartup(kConfigObject);

  doTests(argc ? argv[1] : "");
  // Don't put tests in main because we need to ensure that no KstObjects
  // remain past the exit handler

  exitHelper(); // need to run it here before kapp goes away in some cases.
  if (rc == KstTestSuccess) {
    printf("All tests passed!\n");
  }
  return -rc;
}

// vim: ts=2 sw=2 et
