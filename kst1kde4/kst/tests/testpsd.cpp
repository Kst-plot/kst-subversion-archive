/*
 *  Copyright 2006, The University of Toronto
 *  Licensed under GPL.
 */

#include "ksttestcase.h"
#include "kstpsd.h"
#include "kstdataobject.h"

#include <kstdataobjectcollection.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <qdir.h>
#include <qfile.h>


static void exitHelper() {
  KST::vectorList.clear();
//  KST::scalarList.clear();
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

QDomDocument makeDOMElement(const QString& tag, const QString& val) {
// Should be some boundary checking in the constructor.
  QDomDocument psdDOM("psddocument");
  QDomElement psdElement, child;
  QDomText text;

  psdElement = psdDOM.createElement("psdDOMTest");

  child = psdDOM.createElement("tag");
  text = psdDOM.createTextNode(tag);
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("vectag");
  text = psdDOM.createTextNode(val);
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("sampRate");
  text = psdDOM.createTextNode("128");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("average");
  text = psdDOM.createTextNode("1");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("fiftLen");
  text = psdDOM.createTextNode("5");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("adopize");
  text = psdDOM.createTextNode("0");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("apodizefxn");
  text = psdDOM.createTextNode("WindowOriginal");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("gaussiansigma");
  text = psdDOM.createTextNode("0.01");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("removeMean");
  text = psdDOM.createTextNode("1");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("VUnits");
  text = psdDOM.createTextNode("vUnits");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("RUnits");
  text = psdDOM.createTextNode("rUnits");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("output");
  text = psdDOM.createTextNode("PSDAmplitudeSpectralDensity");
  child.appendChild(text);
  psdElement.appendChild(child);

  child = psdDOM.createElement("interpolateHoles");
  text = psdDOM.createTextNode("1");
  child.appendChild(text);
  psdElement.appendChild(child);

  psdDOM.appendChild(psdElement);

  return psdDOM;
}

void doTests() {

  KstVectorPtr vp = new KstVector(KstObjectTag("tempVector"), 10);
  for (int i = 0; i < 10; i++){
    vp->value()[i] = i;
  }

  KstPSDPtr psd = new KstPSD(QString("psdTest"), vp, 0.0, false, 10, false, false, QString("vUnits"), QString("rUnits"), WindowUndefined, 0.0, PSDUndefined);
  doTest(psd->tagName() == "psdTest");
  doTest(psd->vTag() == "tempVector");
  doTest(psd->output() == PSDUndefined);
  doTest(!psd->apodize());
  doTest(!psd->removeMean());
  doTest(!psd->average());
  doTest(psd->freq() == 0.0);
  doTest(psd->apodizeFxn() == WindowUndefined);
  doTest(psd->gaussianSigma() == 0);
  KstVectorPtr vpVX = psd->vX();
  KstVectorPtr vpVY = psd->vY();

  // until we call update the x and y vectors will be uninitialised and
  // and so they should be of length 1 and the value of vpVX[0] and 
  // vpVX[0] should be NAN...
  doTestV(QString("vpVX->length()"), vpVX->length(), 1);
  doTestV(QString("vpVY->length()"), vpVY->length(), 1);
  doTestV(QString("vpVX->length()"), isnan(vpVX->value()[0]), 1);
  doTestV(QString("vpVY->length()"), isnan(vpVY->value()[0]), 1);

  doTest(psd->update(0) == KstObject::UPDATE);

  for(int j = 0; j < vpVX->length(); j++){
      doTest(vpVX->value()[j] == 0);
  }

  psd->setOutput(PSDAmplitudeSpectralDensity);
  psd->setApodize(true);
  psd->setRemoveMean(true);
  psd->setAverage(true);
  psd->setFreq(0.1);
  psd->setApodizeFxn(WindowOriginal);
  psd->setGaussianSigma(0.2);

  doTest(psd->tagName() == "psdTest");
  doTest(psd->vTag() == "tempVector");
  doTest(psd->output() == PSDAmplitudeSpectralDensity);
  doTest(psd->apodize());
  doTest(psd->removeMean());
  doTest(psd->average());
  doTest(psd->freq() == 0.1);
  doTest(psd->apodizeFxn() == WindowOriginal);
  doTest(psd->gaussianSigma() == 0.2);
  
//   doTest(psd->update(0) == KstObject::UPDATE);
//   QString ps = "PSD: " + psd->vTag();
//   doTest(psd->propertyString() == ps);
//    doTest(!psd->curveHints().curveName() == "");
//   printf("Curve name [%s]", kstCHL[0].curveName());
//   printf("X Vector name [%s]", kstCHL[0].xVectorName());
//   printf("Y Vector name [%s]", kstCHL[0].yVectorName());

  KTempFile tf(locateLocal("tmp", "kst-csd"), "txt");
  QFile *qf = tf.file();
  QTextStream ts(qf);
  psd->save(ts, "");
  QFile::remove(tf.name());

  QDomNode n = makeDOMElement("psdDOMPsd", "psdDOMVector").firstChild();
  QDomElement e = n.toElement();
  KstPSDPtr psdDOM = new KstPSD(e);

  doTest(psdDOM->tagName() == "psdDOMPsd");
  doTest(psdDOM->output() == PSDAmplitudeSpectralDensity);
  doTest(psdDOM->apodize());
  doTest(psdDOM->removeMean());
  doTest(psdDOM->average());
  doTest(psdDOM->freq() == 128);
  doTest(psdDOM->apodizeFxn() == WindowOriginal);
  doTest(psdDOM->gaussianSigma() == 0.01);

//   KstVectorPtr vpVX = psdDOM->vX();
//   for(int j = 0; j < vpVX->length(); j++){
//       printf("[%d][%lf]", j, vpVX->value()[j]);
//   }
//   KstVectorPtr vpVY = psdDOM->vY();
}


int main(int argc, char **argv) {
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  atexit(exitHelper);

  KCmdLineArgs::init(argc, argv, "testpsd", "testpsd", "testpsd", "1.0.0.0");
  KApplication app(false, false);

  doTests();
  // Don't put tests in main because we need to ensure that no KstObjects
  // remain past the exit handler

  exitHelper(); // need to run it here before kapp goes away in some cases.
  if (rc == KstTestSuccess) {
    printf("All tests passed!\n");
  }

  return -rc;
}
