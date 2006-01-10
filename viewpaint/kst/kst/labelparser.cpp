/***************************************************************************
                             labelparser.cpp
                             ----------------
    begin                : Dec 14 2004
                           Copyright (C) 2004, The University of Toronto
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

#include "labelparser.h"

#include <assert.h>

#include <qstring.h>

using namespace Label;

Chunk::Chunk(Chunk *parent, VOffset dir, bool group)
: next(0L), prev(0L), up(0L), down(0L), scalar(false), group(group), linebreak(false), tab(false), vector(false), vOffset(dir) {
  assert(parent || vOffset == None);
  if (parent) {  // attach and inherit
    switch (vOffset) {
      case None:
        while (parent->next) {
          parent = parent->next;
        }
        parent->next = this;
        break;
      case Up:
        assert(!parent->up);
        parent->up = this;
        break;
      case Down:
        assert(!parent->down);
        parent->down = this;
        break;
    }
    prev = parent;
  }
}


Chunk::~Chunk() {
  // These are set to 0 by the child if they're non-zero
  delete next;
  delete up;
  delete down;
  if (prev) {
    switch (vOffset) {
      case None:
        prev->next = 0L;
        break;
      case Up:
        prev->up = 0L;
        break;
      case Down:
        prev->down = 0L;
        break;
    }
    prev = 0L;
  }
}


bool Chunk::locked() const {
  return scalar || group || linebreak || tab || vector;
}


Parsed::Parsed() : chunk(0L) {
}


Parsed::~Parsed() {
  delete chunk;
  chunk = 0L;
}


inline void setNormalChar(QChar c, Chunk **tail) {
  if (*tail && !(*tail)->locked()) {
    (*tail)->text += c;
  } else {
    Chunk *f = new Chunk(*tail);
    f->text += c;
    *tail = f;
  }
}


#define EXPAND_GREEK(L_U, L_L, REST, SKIP, UCODE)    \
  case L_L:                                   \
    x=0x20;                                   \
  case L_U:                                   \
    if (txt.mid(from + 1).startsWith(REST)) { \
      *skip = SKIP;                           \
      setNormalChar(QChar(UCODE+x), tail);    \
      return true;                            \
    }                                         \
  break;


inline bool parseOutChar(const QString& txt, uint from, int *skip, Chunk **tail) {
  QChar c = txt[from];
  bool upper = false;
  *skip = 1;
  short x = 0;

  switch (c.unicode()) {
    EXPAND_GREEK('B', 'b', "eta",   4, 0x392)
    EXPAND_GREEK('D', 'd', "elta",  5, 0x394)
    EXPAND_GREEK('Z', 'z', "eta",   4, 0x396)
    EXPAND_GREEK('K', 'k', "appa",  5, 0x39a)
    EXPAND_GREEK('M', 'm', "u",     2, 0x39c)
    EXPAND_GREEK('X', 'x', "i",     2, 0x39e)
    EXPAND_GREEK('R', 'r', "ho",    3, 0x3a1)
    EXPAND_GREEK('U', 'u', "psilon",7, 0x3a5)


    case 'a':
      x = 0x20;
    case 'A':
      if (txt.mid(from + 1).startsWith("lpha")) {
        *skip = 5;
        setNormalChar(QChar(0x391+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("pprox")) {
        *skip = 6;
        setNormalChar(QChar(0x2248), tail);
        return true;
      }
      break;


    case 'c':
      x = 0x20;
    case 'C':
      if (txt.mid(from + 1).startsWith("hi")) {
        *skip = 3;
        setNormalChar(QChar(0x3a7+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("dot")) {
        *skip = 4;
        setNormalChar(QChar(0x2219), tail);
        return true;
      }
      break;

    case 'e':
      x = 0x20;
    case 'E':
      if (txt.mid(from + 1).startsWith("psilon")) {
        *skip = 7;
        setNormalChar(QChar(0x395+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("ta")) {
        *skip = 3;
        setNormalChar(QChar(0x397+x), tail);
        return true;
      }
      break;

    case 'g':
      x = 0x20;
    case 'G':
      if (txt.mid(from + 1).startsWith("amma")) {
        *skip = 5;
        setNormalChar(QChar(0x393+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("eq")) {
        *skip = 3;
        setNormalChar(QChar(0x2265), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("e")) {
        *skip = 2;
        setNormalChar(QChar(0x2265), tail);
        return true;
      }
      break;

    case 'i':
      x = 0x20;
    case 'I':
      if (txt.mid(from + 1).startsWith("ota")) {
        *skip = 4;
        setNormalChar(QChar(0x399+x), tail);
        return true;
      } else if (!upper && txt.mid(from + 1).startsWith("nf")) {
        *skip = 3;
        setNormalChar(QChar(0x221E), tail);
        return true;
      } else if (!upper && txt.mid(from + 1).startsWith("nt")) {
        *skip = 3;
        setNormalChar(QChar(0x222B), tail);
        return true;
      }
      break;

    case 'l':
      x = 0x20;
    case 'L':
      if (txt.mid(from + 1).startsWith("ambda")) {
        *skip = 6;
        setNormalChar(QChar(0x39b+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("eq")) {
        *skip = 3;
        setNormalChar(QChar(0x2264), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("e")) {
        *skip = 2;
        setNormalChar(QChar(0x2264), tail);
        return true;
      }
      break;

    case 'n':
      x = 0x20;
    case 'N':
      if (txt.mid(from + 1).startsWith("u")) {
        *skip = 2;
        setNormalChar(QChar(0x39D+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("e")) {
          *skip = 2;
          setNormalChar(QChar(0x2260), tail);
          return true;
        } else {
        *skip = 1;
        if (!*tail || !(*tail)->text.isEmpty() || (*tail)->locked()) {
          *tail = new Chunk(*tail);
        }
        (*tail)->linebreak = true;
        return true;
      }
      break;

    case 'o':
      x = 0x20;
    case 'O':
      if (txt.mid(from + 1).startsWith("micron")) {
        *skip = 7;
        setNormalChar(QChar(0x39F+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("mega")) {
        *skip = 5;
        setNormalChar(QChar(0x3A9+x), tail);
        return true;
      }
      break;

    case 'p':
      x = 0x20;
    case 'P':
      if (txt.mid(from + 1).startsWith("i")) {
        *skip = 2;
        setNormalChar(QChar(0x3a0+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("hi")) {
        *skip = 3;
        setNormalChar(QChar(0x3A6+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("si")) {
        *skip = 3;
        setNormalChar(QChar(0x3A8+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("artial")) {
        *skip = 7;
        setNormalChar(QChar(0x2202), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("rod")) {
        *skip = 4;
        setNormalChar(QChar(0x220F), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("m")) {
        *skip = 2;
        setNormalChar(QChar(0xb1), tail);
        return true;
      }
      break;

    case 't':
      x = 0x20;
    case 'T':
      if (txt.mid(from + 1).startsWith("heta")) {
        *skip = 5;
        setNormalChar(QChar(0x398+x), tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("au")) {
        *skip = 3;
        setNormalChar(QChar(0x3A4+x), tail);
        return true;
      } else {
        *skip = 1;
        if (!*tail || !(*tail)->text.isEmpty() || (*tail)->locked()) {
          *tail = new Chunk(*tail);
        }
        (*tail)->tab = true;
        return true;
      }
      break;

    case 's':
      x = 0x20;
    case 'S':
      if (txt.mid(from + 1).startsWith("igma")) {
        *skip = 5;
        setNormalChar(QChar(0x3A3+x), tail);
        return true;
      } else if (!upper && txt.mid(from + 1).startsWith("um")) {
        *skip = 3;
        setNormalChar(QChar(0x2211), tail);
        return true;
      } else if (!upper && txt.mid(from + 1).startsWith("qrt")) {
        *skip = 4;
        setNormalChar(QChar(0x221A), tail);
        return true;
      }
      break;

    default:
      break;
  }

  return false;
}


static Chunk *parseInternal(Chunk *ctail, const QString& txt, uint& start, uint cnt) {
  Chunk *chead = ctail;

  if (ctail->group) {
    ctail = new Chunk(ctail);
  }

  for (uint& i = start; i < cnt; ++i) {
    QChar c = txt[i];
    Chunk::VOffset dir = Chunk::Down;
    switch (c.unicode()) {
      case 0x5c:   // \ /**/
        if (ctail->vOffset != Chunk::None && !ctail->text.isEmpty()) {
          ctail = new Chunk(ctail->prev);
        }
        ++i;
        if (i == cnt) {
          setNormalChar('\\', &ctail);
        } else {
          int skip = 0;
          if (!parseOutChar(txt, i, &skip, &ctail)) {
            setNormalChar(txt[i], &ctail);
          } else {
            i += skip - 1;
          }
        }
        break;
      case 0x5e:   // ^
        dir = Chunk::Up;
      case 0x5f:   // _
        if (ctail->text.isEmpty() && !ctail->group) {
          setNormalChar(c, &ctail);
        } else {
          if (ctail->vOffset != Chunk::None) {
            if (ctail->vOffset != dir) {
              ctail = new Chunk(ctail->prev, dir);
            } else {
              return 0L; // parse error - x^y^z etc
            }
          } else {
            ctail = new Chunk(ctail, dir);
          }
        }
        break;
      case 0x7b:   // {
        if (ctail->text.isEmpty() && !ctail->group) {
          ctail->group = true;
          ctail = parseInternal(ctail, txt, ++i, cnt);
        } else {
          if (ctail->vOffset == Chunk::None) {
            ctail = parseInternal(new Chunk(ctail, Chunk::None, true), txt, ++i, cnt);
          } else {
            ctail = parseInternal(new Chunk(ctail->prev, Chunk::None, true), txt, ++i, cnt);
          }
        }
        if (!ctail) {
          return ctail;
        }
        break;
      case 0x7d:   // }
        if (chead->group) {
          return chead;
        } else {
          setNormalChar(c, &ctail);
        }
        break;
      case '[':
        {
          bool vector = false;
          int vectorIndexStart = -1;
          int vectorIndexEnd = -1;
          int bracketStack = 1;
          int pos = -1;
          bool equation = txt[i + 1] == '=';
          for (uint searchPt = i + 1; bracketStack != 0 && searchPt < cnt; ++searchPt) {
            if (txt[searchPt] == ']') {
              if (--bracketStack == 0) {
                pos = searchPt;
                break;
              } else if (bracketStack == 1 && vector && vectorIndexEnd == -1) {
                vectorIndexEnd = searchPt - 1;
              }
            } else if (txt[searchPt] == '[') {
              ++bracketStack;
              if (!vector && !equation) {
                vector = true;
                vectorIndexStart = searchPt + 1;
              }
            }
          }

          if (pos < 0 || pos == int(i) + 1 /* empty [] */) {
            return 0L;
          }

          if (ctail->locked() || !ctail->text.isEmpty()) {
            if (ctail->vOffset != Chunk::None) {
              ctail = new Chunk(ctail->prev);
            } else {
              ctail = new Chunk(ctail);
            }
          }

          if (vector) {
            ctail->text = txt.mid(i + 1, vectorIndexStart - i - 2).stripWhiteSpace();
            ctail->expression = txt.mid(vectorIndexStart, vectorIndexEnd - vectorIndexStart + 1);
            ctail->vector = true;
          } else {
            ctail->text = txt.mid(i + 1, pos - i - 1).stripWhiteSpace();
            ctail->scalar = true;
          }
          i = uint(pos);
        }
        break;
      default:
        if (ctail->vOffset != Chunk::None && (!ctail->text.isEmpty() || ctail->locked())) {
          ctail = new Chunk(ctail->prev);
        }
        setNormalChar(c, &ctail);
        break;
    }
  }

  return chead;
}


Parsed *Label::parse(const QString& txt, bool interpret) {
  Parsed *parsed = new Parsed;
  Chunk *ctail = parsed->chunk = new Chunk(0L);
  if (!interpret) {
    ctail->text = txt;
    return parsed;
  }

  uint start = 0;
  uint cnt = txt.length();
  Chunk *rc = parseInternal(ctail, txt, start, cnt);
  if (!rc) {
    // Parse error - how to recover?
    delete parsed;
    parsed = 0L;
  }
  return parsed;
}


// vim: ts=2 sw=2 et
