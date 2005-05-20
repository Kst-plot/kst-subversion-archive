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
: next(0L), prev(0L), up(0L), down(0L), symbol(false), scalar(false), group(group), vOffset(dir) {
  assert(parent || vOffset == None);
  if (parent) {  // attach and inherit
    switch (vOffset) {
      case None:
        assert(!parent->next);
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
  return scalar || group;
}


Parsed::Parsed() : chunk(0L) {
}


Parsed::~Parsed() {
  delete chunk;
  chunk = 0L;
}


inline void setSymbolChar(QChar c, Chunk **tail) {
  if (*tail && !(*tail)->locked() && ((*tail)->symbol || (*tail)->text.isEmpty())) {
    (*tail)->text += c;
    (*tail)->symbol = true;
  } else {
    Chunk *f = new Chunk(*tail);
    f->symbol = true;
    f->text += c;
    *tail = f;
  }
}


inline void setNormalChar(QChar c, Chunk **tail) {
  if (*tail && !(*tail)->locked() && !(*tail)->symbol) {
    (*tail)->text += c;
  } else {
    Chunk *f = new Chunk(*tail);
    f->symbol = false;
    f->text += c;
    *tail = f;
  }
}


#define EXPAND_GREEK(L_U, L_L, REST, SKIP)    \
  case L_U:                                   \
  case L_L:                                   \
    if (txt.mid(from + 1).startsWith(REST)) { \
      *skip = SKIP;                           \
      setSymbolChar(c, tail);                 \
      return true;                            \
    }                                         \
  break;


inline bool parseOutChar(const QString& txt, uint from, int *skip, Chunk **tail) {
  QChar c = txt[from];
  bool upper = false;
  *skip = 1;

  switch (c.unicode()) {
    EXPAND_GREEK('A', 'a', "lpha",  5)
    EXPAND_GREEK('B', 'b', "eta",   4)
    EXPAND_GREEK('G', 'g', "amma",  5)
    EXPAND_GREEK('D', 'd', "elta",  5)
    EXPAND_GREEK('Z', 'z', "eta",   4)
    EXPAND_GREEK('K', 'k', "appa",  5)
    EXPAND_GREEK('L', 'l', "ambda", 6)
    EXPAND_GREEK('M', 'm', "u",     2)
    EXPAND_GREEK('X', 'x', "i",     2)
    EXPAND_GREEK('R', 'r', "ho",    3)
    EXPAND_GREEK('C', 'c', "hi",    3)
    EXPAND_GREEK('U', 'u', "psilon",7)

    case 'E':
      upper = true;
    case 'e':
      if (txt.mid(from + 1).startsWith("psilon")) {
        *skip = 7;
        setSymbolChar(c, tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("ta")) {
        *skip = 3;
        c = upper ? 'H' : 'h';
        setSymbolChar(c, tail);
        return true;
      }
      break;

    case 'N':
      upper = true;
    case 'n':
      if (txt.mid(from + 1).startsWith("u")) {
        *skip = 2;
        setSymbolChar(c, tail);
        return true;
      } else {
        *skip = 1;
        setNormalChar('\n', tail);
        return true;
      }
      break;

    case 'T':
      upper = true;
    case 't':
      if (txt.mid(from + 1).startsWith("heta")) {
        *skip = 5;
        c = upper ? 'Q' : 'q';
        setSymbolChar(c, tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("au")) {
        *skip = 3;
        setSymbolChar(c, tail);
        return true;
      } else {
        *skip = 1;
        setNormalChar('\t', tail);
        return true;
      }
      break;

    case 'I':
      upper = true;
    case 'i':
      if (txt.mid(from + 1).startsWith("ota")) {
        *skip = 4;
        setSymbolChar(c, tail);
        return true;
      } else if (!upper && txt.mid(from + 1).startsWith("nf")) {
        *skip = 3;
        c = 165;
        setSymbolChar(c, tail);
        return true;
      }
      break;

    case 'O':
      upper = true;
    case 'o':
      if (txt.mid(from + 1).startsWith("micron")) {
        *skip = 7;
        setSymbolChar(c, tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("mega")) {
        *skip = 5;
        c = upper ? 'W' : 'w';
        setSymbolChar(c, tail);
        return true;
      }
      break;

    case 'P':
      upper = true;
    case 'p':
      if (txt.mid(from + 1).startsWith("i")) {
        *skip = 2;
        setSymbolChar(c, tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("hi")) {
        *skip = 3;
        c = upper ? 'F' : 'f';
        setSymbolChar(c, tail);
        return true;
      } else if (txt.mid(from + 1).startsWith("si")) {
        *skip = 3;
        c = upper ? 'Y' : 'y';
        setSymbolChar(c, tail);
        return true;
      }
      break;

    case 'S':
      upper = true;
    case 's':
      if (txt.mid(from + 1).startsWith("igma")) {
        *skip = 5;
        setSymbolChar(c, tail);
        return true;
      } else if (!upper && txt.mid(from + 1).startsWith("um")) {
        *skip = 3;
        c = 229;
        setSymbolChar(c, tail);
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
        if (ctail->text.isEmpty()) {
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
          int bracketStack = 1;
          int pos = -1;
          for (uint searchPt = i + 1; bracketStack != 0 && searchPt < cnt; ++searchPt) {
            if (txt[searchPt] == ']') {
              if (--bracketStack == 0) {
                pos = searchPt;
                break;
              }
            } else if (txt[searchPt] == '[') {
              ++bracketStack;
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

          ctail->text = txt.mid(i + 1, pos - i - 1);
          ctail->scalar = true;
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
