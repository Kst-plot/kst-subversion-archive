/*
 *  Copyright 2005, The University of Toronto
 *  Licensed under GPL.
 */

#include "ksttestcase.h"
#include <kstdataobjectcollection.h>

static void exitHelper() {
  KST::matrixList.clear();
  KST::scalarList.clear();
  KST::dataObjectList.clear();
}


int rc = KstTestSuccess;

#define doTest(x) testAssert(x, QString("Line %1").arg(__LINE__))
#define doTestD(x, y) testAssert(x, QString("%1: %2").arg(__LINE__).arg(y))

void testAssert(bool result, const QString& text = "Unknown") {
  if (!result) {
    KstTestFailed();
    printf("Test [%s] failed.\n", text.latin1());
  }
}

void doTests() {
  bool ok = true;

  //basic default constructor values
  KstMatrixPtr m1 = new KstMatrix(KstObjectTag(QString::null));
  doTest(m1->tagName().startsWith("Anonymous"));
  doTest(m1->sampleCount() == 0);
  doTest(m1->minValue() == 0);
  doTest(m1->maxValue() == 0);
  doTest(m1->value(0, 0, &ok) == 0);
  doTest(!ok);
  doTest(m1->value(10, 10, &ok) == 0); //should be outside the boundaries.
  doTest(!ok);
  doTest(m1->sampleCount() == 0);
  doTest(m1->meanValue() == 0);

  //basic symetrical matrix
  KstMatrixPtr m2 = new KstMatrix(KstObjectTag("Symetrical"), 0L, 3, 3);
  doTest(m2->tagName() == "Symetrical");

  doTest(m2->resize(3, 3, true));

  doTest(!m2->editable());
  m2->setEditable(true);

  doTest(m2->editable());

  doTest(m2->xNumSteps() == 3);
  doTest(m2->yNumSteps() == 3);
  doTest(m2->minX() == 0);
  doTest(m2->minY() == 0);
  doTest(m2->xStepSize() == 1);
  doTest(m2->yStepSize() == 1);
  doTest(m2->sampleCount() == 9);

  doTest(m2->setValueRaw(1, 1, 5));
  ok = true;
  doTest(m2->value(1, 1, &ok) == 5);
  doTest(ok);

  m2->blank();

  m2->change(m2->tag(), 3, 3, 0, 0, 0, 0); //should not be legal
  doTest(m2->xNumSteps() == 3);
  doTest(m2->yNumSteps() == 3);
  doTest(m2->minX() == 0);
  doTest(m2->minY() == 0);
  doTest(m2->xStepSize() == 0);
  doTest(m2->yStepSize() == 0);
  doTest(m2->sampleCount() == 9);

  doTest(!m2->setValue(0, 0, 1.0));
  ok = true;
  doTest(m2->value(0, 0, &ok) == 0.0);
  doTest(!ok);

  doTest(!m2->setValue(1, 1, 5.0));
  doTest(m2->value(1, 1) != 5.0);
  doTest(m2->setValueRaw(2, 2, 6.0)); //fails

  KstMatrixPtr um1 = new KstMatrix(KstObjectTag("Unity"), 0L, 3, 3, 0, 0, 1, 1);
  um1->setEditable(true);
  um1->zero();
  doTest(!um1->setValue(0, 0, 1.0));
  doTest(!um1->setValue(1, 1, 1.0));
  doTest(!um1->setValue(2, 2, 1.0));

  doTest(um1->resize(3, 3, false));
  um1->zero();
  doTest(um1->setValue(0, 0, 1.0));
  doTest(um1->setValue(1, 1, 1.0));
  doTest(um1->setValue(2, 2, 1.0));

  // calling resize on a matrix does not retain the correct values
  // for matrix entries. i.e. taking a 3x3 matrix and resizing
  // to a 2x2 matrix does not mean matrix[0][0], matrix[0][1],
  // matrix[1][0], matrix[1][1] are the same before and after the resize.
  // The resulting values should properly be undefined...
  doTest(um1->resize(2, 2, true));
  doTest(um1->sampleCount() == 4); 

  doTest(um1->value(0, 0, &ok) == 1.0);
  doTest(ok);
  doTest(um1->value(0, 1, &ok) == 0);
  doTest(ok);
  doTest(um1->value(0, 2, &ok) == 0);
  doTest(!ok);
  doTest(um1->value(1, 0, &ok) == 0);
  doTest(ok);
  doTest(um1->value(1, 1, &ok) == 0);
  doTest(ok);
  doTest(um1->value(1, 2, &ok) == 0);
  doTest(!ok);

  doTest(um1->resize(4, 4, false));
  doTest(um1->value(0, 0, &ok) == 1.0);
  doTest(ok);
  doTest(um1->value(0, 1, &ok) == 0);
  doTest(ok);
  doTest(um1->value(0, 2, &ok) == 0);
  doTest(ok);
  doTest(um1->value(1, 0, &ok) == 0);
  doTest(ok);
  doTest(um1->value(1, 1, &ok) == 0);
  doTest(ok);
  doTest(um1->value(1, 2, &ok) == 0);
  doTest(ok);

  doTest(um1->resize(3, 3, true));
  doTest(um1->setValue(0, 0, 1.716299));
  doTest(um1->setValue(0, 1, -0.485527));
  doTest(um1->setValue(0, 2, -0.288690));
  doTest(um1->setValue(1, 0, 1.716299));
  doTest(um1->setValue(1, 1, NAN));
  doTest(um1->setValue(1, 2, -0.274957));
  doTest(um1->setValue(2, 0, 1.711721));
  doTest(um1->setValue(2, 1, -0.485527));
  doTest(um1->setValue(2, 2, -0.293267));

  doTest(um1->value(0, 0) == 1.716299);
  doTest(um1->value(0, 1) ==  -0.485527);
  doTest(um1->value(0, 2) == -0.288690);
  doTest(um1->value(1, 0) == 1.716299);
  doTest(um1->value(1, 1) == 0);
  doTest(um1->value(1, 2) == -0.274957);
  doTest(um1->value(2, 0) == 1.711721);
  doTest(um1->value(2, 1) == -0.485527);
  doTest(um1->value(2, 2) == -0.293267);

  doTest(um1->minValue() == 0);
  doTest(um1->maxValue() == 0);

  KstMatrixPtr sm = new KstMatrix(KstObjectTag("Spike"), 0L, 2, 2, 0, 0, 1, 1);
  
  sm->setEditable(true);
  doTest(sm->resize(2, 2, false));
  doTest(sm->xNumSteps() == 2);
  doTest(sm->yNumSteps() == 2);

  doTest(sm->setValueRaw(0, 0, 0.0));
  doTest(sm->setValueRaw(0, 1, 0.1));
  doTest(sm->setValueRaw(1, 0, 1.0));
  doTest(sm->setValueRaw(1, 1, 1.1));

  sm->calcNoSpikeRange(0);
  doTest(sm->minValueNoSpike() == 0.0);
  doTest(sm->maxValueNoSpike() == 0.0);

  sm->calcNoSpikeRange(-100);
  doTest(sm->minValueNoSpike() == 0.0);
  doTest(sm->maxValueNoSpike() == 0.0);
  
  sm->calcNoSpikeRange(0.9);
  doTest(sm->minValueNoSpike() >= 1E+300 );
  doTest(sm->maxValueNoSpike() <= -1E+300);
}


int main(int argc, char **argv) {
  atexit(exitHelper);

  KApplication app(argc, argv, "testmatrix", false, false);

  doTests();
  // Don't put tests in main because we need to ensure that no KstObjects
  // remain past the exit handler

  exitHelper(); // need to run it here before kapp goes away in some cases.
  if (rc == KstTestSuccess) {
    printf("All tests passed!\n");
  }
  return -rc;
}
