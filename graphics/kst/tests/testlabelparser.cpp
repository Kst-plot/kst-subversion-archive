/*
 *  Copyright 2004, The University of Toronto
 *  Licensed under GPL.
 */

#include "ksttestcase.h"
#include <labelparser.h>

static void exitHelper() {
  KST::vectorList.clear();
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
  Label::Parsed *parsed = Label::parse("");
  doTestD(parsed != 0L, "Empty label parsed");
  doTest(parsed->chunk->prev == 0L);
  doTest(parsed->chunk->next == 0L);
  doTest(parsed->chunk->text == QString::null);
  doTest(parsed->chunk->vOffset == Label::Chunk::None);
  doTest(parsed->chunk->symbol == false);
  delete parsed;

  parsed = Label::parse("a");
  doTest(parsed != 0L);
  doTest(parsed->chunk->prev == 0L);
  doTest(parsed->chunk->next == 0L);
  doTest(parsed->chunk->text == "a");
  doTest(parsed->chunk->vOffset == Label::Chunk::None);
  doTest(parsed->chunk->symbol == false);
  delete parsed;

  parsed = Label::parse("\\tau");
  doTest(parsed != 0L);
  doTest(parsed->chunk->prev == 0L);
  doTest(parsed->chunk->next == 0L);
  doTest(parsed->chunk->text == "t");
  doTest(parsed->chunk->vOffset == Label::Chunk::None);
  doTest(parsed->chunk->symbol == true);
  delete parsed;

  parsed = Label::parse("\\t");
  doTest(parsed != 0L);
  doTest(parsed->chunk->text == "\t");
  doTest(parsed->chunk->vOffset == Label::Chunk::None);
  doTest(parsed->chunk->symbol == false);
  delete parsed;

  parsed = Label::parse("\\n");
  doTest(parsed != 0L);
  doTest(parsed->chunk->text == "\n");
  doTest(parsed->chunk->vOffset == Label::Chunk::None);
  doTest(parsed->chunk->symbol == false);
  delete parsed;

  parsed = Label::parse("\\tau\\Theta");
  doTest(parsed != 0L);
  doTest(parsed->chunk->next == 0L);
  doTest(parsed->chunk->text == "tQ");
  doTest(parsed->chunk->symbol == true);
  delete parsed;

  Label::Chunk *c = 0L;

  parsed = Label::parse("\\taufoo bar\\n\\Theta");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->next != 0L);
  doTest(c->text == "t");
  doTest(c->symbol == true);
  c = c->next;
  doTest(c->prev != 0L);
  doTest(c->next != 0L);
  doTest(c->text == "foo bar\n");
  doTest(c->symbol == false);
  c = c->next;
  doTest(c->prev != 0L);
  doTest(c->next == 0L);
  doTest(c->text == "Q");
  doTest(c->symbol == true);
  delete parsed;

  parsed = Label::parse("x^y^z");
  doTest(parsed == 0L);

  parsed = Label::parse("x_y_z");
  doTest(parsed == 0L);

  parsed = Label::parse("x^y_z");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->prev == 0L);
  doTest(c->next == 0L);
  doTest(c->up != 0L);
  doTest(c->down != 0L);
  doTest(c->text == "x");
  doTest(c->symbol == false);
  doTest(c->vOffset == Label::Chunk::None);
  c = c->up;
  doTest(c->prev != 0L);
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "y");
  doTest(c->symbol == false);
  doTest(c->vOffset == Label::Chunk::Up);
  c = c->prev->down;
  doTest(c->prev != 0L);
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "z");
  doTest(c->symbol == false);
  doTest(c->vOffset == Label::Chunk::Down);
  delete parsed;

  parsed = Label::parse("x^{y+1}_{z-1}");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->prev == 0L);
  doTest(c->next == 0L);
  doTest(c->up != 0L);
  doTest(c->down != 0L);
  doTest(c->text == "x");
  doTest(c->symbol == false);
  doTest(c->vOffset == Label::Chunk::None);
  c = c->up;
  doTest(c->prev != 0L);
  doTest(c->next != 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->vOffset == Label::Chunk::Up);
  c = c->next;
  doTest(c->text == "y+1");
  doTest(c->symbol == false);
  doTest(c->vOffset == Label::Chunk::None);
  c = c->prev->prev->down;
  doTest(c->prev != 0L);
  doTest(c->next != 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->vOffset == Label::Chunk::Down);
  c = c->next;
  doTest(c->text == "z-1");
  doTest(c->symbol == false);
  doTest(c->vOffset == Label::Chunk::None);
  delete parsed;

  parsed = Label::parse("x\\^y");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->prev == 0L);
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "x^y");
  doTest(c->vOffset == Label::Chunk::None);
  delete parsed;

  parsed = Label::parse("x^{y^{q+1} + 1}_{z-1}");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->prev == 0L);
  doTest(c->next == 0L);
  doTest(c->up != 0L);
  doTest(c->down != 0L);
  doTest(c->text == "x");
  doTest(c->vOffset == Label::Chunk::None);
  c = c->up;
  doTest(c->prev != 0L);
  doTest(c->next != 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->group);
  doTest(c->vOffset == Label::Chunk::Up);
  c = c->next;
  doTest(c->text == "y");
  doTest(c->prev != 0L);
  doTest(c->next != 0L);
  doTest(c->up != 0L);
  doTest(c->down == 0L);
  doTest(c->vOffset == Label::Chunk::None);
  c = c->up;
  doTest(c->prev != 0L);
  doTest(c->next != 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->group);
  doTest(c->vOffset == Label::Chunk::Up);
  c = c->next;
  doTest(c->prev != 0L);
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "q+1");
  doTest(c->vOffset == Label::Chunk::None);
  c = c->prev->prev->next;
  doTest(c->prev != 0L);
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == " + 1");
  doTest(c->vOffset == Label::Chunk::None);
  c = c->prev->prev->prev->down;
  doTest(c->prev != 0L);
  doTest(c->next != 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->group);
  doTest(c->vOffset == Label::Chunk::Down);
  c = c->next;
  doTest(c->prev != 0L);
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "z-1");
  doTest(c->vOffset == Label::Chunk::None);
  delete parsed;

  parsed = Label::parse("{2*2}");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->next != 0L);
  doTest(c->vOffset == Label::Chunk::None);
  c = c->next;
  doTest(c->text == "2*2");
  doTest(c->vOffset == Label::Chunk::None);
  delete parsed;

  parsed = Label::parse("x^100");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->next != 0L);
  doTest(c->up != 0L);
  doTest(c->text == "x");
  doTest(c->vOffset == Label::Chunk::None);
  c = c->next;
  doTest(c->next == 0L);
  doTest(c->text == "00");
  doTest(c->vOffset == Label::Chunk::None);
  c = c->prev->up;
  doTest(c->next == 0L);
  doTest(c->text == "1");
  doTest(c->vOffset == Label::Chunk::Up);
  delete parsed;

  parsed = Label::parse("x^100*200");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->next != 0L);
  doTest(c->up != 0L);
  doTest(c->text == "x");
  doTest(c->vOffset == Label::Chunk::None);
  c = c->next;
  doTest(c->next == 0L);
  doTest(c->text == "00*200");
  doTest(c->vOffset == Label::Chunk::None);
  c = c->prev->up;
  doTest(c->next == 0L);
  doTest(c->text == "1");
  doTest(c->vOffset == Label::Chunk::Up);
  delete parsed;

  parsed = Label::parse("[ a ^label ]");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == " a ^label ");
  doTest(c->scalar);
  delete parsed;

  parsed = Label::parse("[a][b]");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->next != 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "a");
  doTest(c->scalar);
  c = c->next;
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "b");
  doTest(c->scalar);
  delete parsed;

  parsed = Label::parse("[a]*[b]");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->next != 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "a");
  doTest(c->scalar);
  c = c->next;
  doTest(c->next != 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "*");
  doTest(!c->scalar);
  c = c->next;
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "b");
  doTest(c->scalar);
  delete parsed;

  parsed = Label::parse("[x]^[a]_[b][c]");
  doTest(parsed != 0L);
  c = parsed->chunk;
  doTest(c->next != 0L);
  doTest(c->up != 0L);
  doTest(c->down != 0L);
  doTest(c->text == "x");
  doTest(c->scalar);
  c = c->up;
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "a");
  doTest(c->scalar);
  c = c->prev->down;
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "b");
  doTest(c->scalar);
  c = c->prev->next;
  doTest(c->next == 0L);
  doTest(c->up == 0L);
  doTest(c->down == 0L);
  doTest(c->text == "c");
  doTest(c->scalar);
  delete parsed;

}


int main(int argc, char **argv) {
  atexit(exitHelper);

  KApplication app(argc, argv, "testlabelparser", false, false);

  doTests();
  // Don't put tests in main because we need to ensure that no KstObjects
  // remain past the exit handler

  exitHelper(); // need to run it here before kapp goes away in some cases.
  if (rc == KstTestSuccess) {
    printf("All tests passed!\n");
  }
  return -rc;
}

// vim: ts=2 sw=2 et
