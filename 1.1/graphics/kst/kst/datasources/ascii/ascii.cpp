/***************************************************************************
                     ascii.cpp  -  ASCII file data source
                             -------------------
    begin                : Fri Oct 17 2003
    copyright            : (C) 2003 The University of Toronto
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

#include "ascii.h"
#include "asciiconfig.h"

#include <kcombobox.h>
#include <kdebug.h>

#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qstylesheet.h>

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#define DEFAULT_DELIMITERS "#/c!;"
#define DEFAULT_COLUMN_WIDTH 16

class AsciiSource::Config {
  public:
    Config() {
      _indexInterpretation = Unknown;
      _indexVector = "INDEX";
      _delimiters = DEFAULT_DELIMITERS;
      _columnType = Whitespace;
      _columnWidth = DEFAULT_COLUMN_WIDTH;
      _dataLine = 0;
      _readFields = false;
      _fieldsLine = 0;
    }

    void read(KConfig *cfg, const QString& fileName = QString::null) {
      cfg->setGroup("ASCII General");
      _fileNamePattern = cfg->readEntry("Filename Pattern", QString::null);
      _delimiters = cfg->readEntry("Comment Delimiters", "#/c!;");
      _indexInterpretation = (Interpretation)cfg->readNumEntry("Default INDEX Interpretation", Unknown);
      _columnType = (ColumnType)cfg->readNumEntry("Column Type", Whitespace);
      _columnDelimiter = cfg->readEntry("Column Delimiter", QString::null);
      _columnWidth = cfg->readNumEntry("Column Width", DEFAULT_COLUMN_WIDTH);
      _dataLine = cfg->readNumEntry("Data Start", 0);
      _readFields = cfg->readBoolEntry("Read Fields", false);
      _fieldsLine = cfg->readNumEntry("Fields Line", 0);
      if (!fileName.isEmpty()) {
        cfg->setGroup(fileName);
        _delimiters = cfg->readEntry("Comment Delimiters", _delimiters);
        _indexInterpretation = (Interpretation)cfg->readNumEntry("Default INDEX Interpretation", _indexInterpretation);
        _columnType = (ColumnType)cfg->readNumEntry("Column Type", _columnType);
        _columnDelimiter = cfg->readEntry("Column Delimiter", _columnDelimiter);
        _columnWidth = cfg->readNumEntry("Column Width", _columnWidth);
        _dataLine = cfg->readNumEntry("Data Start", _dataLine);
        _readFields = cfg->readBoolEntry("Read Fields", _readFields);
        _fieldsLine = cfg->readNumEntry("Fields Line", _fieldsLine);
      }
      _delimiters = QRegExp::escape(_delimiters);
    }

    QString _delimiters;
    QString _indexVector;
    QString _fileNamePattern;
    enum Interpretation { Unknown = 0, INDEX, CTime, Seconds, IntEnd = 0xffff };
    Interpretation _indexInterpretation;
    enum ColumnType { Whitespace = 0, Fixed, Custom, ColEnd = 0xffff };
    ColumnType _columnType;
    QString _columnDelimiter;
    int _columnWidth;
    int _dataLine;
    bool _readFields;
    int _fieldsLine;

    void save(QTextStream& str, const QString& indent) {
      if (_indexInterpretation != AsciiSource::Config::Unknown) {
        str << indent << "<index vector=\"" << QStyleSheet::escape(_indexVector) << "\" interpretation=\"" << int(_indexInterpretation) << "\"/>" << endl;
      }
      str << indent << "<comment delimiters=\"" << QStyleSheet::escape(_delimiters) << "\"/>" << endl;
      str << indent << "<columns type=\"" << int(_columnType) << "\"";
      if (_columnType == Fixed) {
        str << " width=\"" << _columnWidth << "\"";
      } else if (_columnType == Custom) {
        str << " delimiters=\"" << QStyleSheet::escape(_columnDelimiter) << "\"";
      }
      str << "/>" << endl;
      str << indent << "<header start=\"" << _dataLine << "\"";
      if (_readFields) {
        str << " fields=\"" << _fieldsLine << "\"";
      }
      str << "/>" << endl;
    }

    void load(const QDomElement& e) {
       QDomNode n = e.firstChild();
       while (!n.isNull()) {
         QDomElement e = n.toElement();
         if (!e.isNull()) {
           if (e.tagName() == "index") {
             if (e.hasAttribute("vector")) {
               _indexVector = e.attribute("vector");
             }
             if (e.hasAttribute("interpretation")) {
               _indexInterpretation = Interpretation(e.attribute("interpretation").toInt());
             }
           } else if (e.tagName() == "comment") {
             if (e.hasAttribute("delimiters")) {
               _delimiters = e.attribute("delimiters");
             }
           } else if (e.tagName() == "columns") {
             if (e.hasAttribute("type")) {
               _columnType = ColumnType(e.attribute("type").toInt());
             }
             if (e.hasAttribute("width")) {
               _columnWidth = e.attribute("width").toInt();
             }
             if (e.hasAttribute("delimiters")) {
               _columnDelimiter = e.attribute("delimiters");
             }
           } else if (e.tagName() == "header") {
             if (e.hasAttribute("start")) {
               _dataLine = e.attribute("start").toInt();
             }
             if (e.hasAttribute("fields")) {
               _fieldsLine = e.attribute("fields").toInt();
             }
           }
         }
         n = n.nextSibling();
       }
    }
};


AsciiSource::AsciiSource(KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e)
: KstDataSource(cfg, filename, type), _rowIndex(0L), _config(0L), _tmpBuf(0L), _tmpBufSize(0) {
  _valid = false;
  _haveHeader = false;
  _fieldListComplete = false;
  if (!type.isEmpty() && type != "ASCII") {
    return;
  }

  _config = new AsciiSource::Config;
  _config->read(cfg, filename);
  if (!e.isNull()) {
    _config->load(e);
  }

  _valid = true;
  update();
}


AsciiSource::~AsciiSource() {
  if (_tmpBuf) {
    free(_tmpBuf);
    _tmpBuf = 0L;
    _tmpBufSize = 0;
  }

  if (_rowIndex) {
    free(_rowIndex);
    _rowIndex = 0L;
    _numLinesAlloc = 0;
  }

  delete _config;
  _config = 0L;
}


bool AsciiSource::reset() {
  if (_tmpBuf) {
    free(_tmpBuf);
    _tmpBuf = 0L;
    _tmpBufSize = 0;
  }

  if (_rowIndex) {
    free(_rowIndex);
    _rowIndex = 0L;
    _numLinesAlloc = 0;
  }

  _haveHeader = false;
  _fieldListComplete = false;
  _fieldList.clear();

  update(); // Yuck - same problem as in the constructor presently.

  return true;
}


bool AsciiSource::initRowIndex() {
  if (!_rowIndex) {
    _rowIndex = (int *)malloc(32768 * sizeof(int));
    _numLinesAlloc = 32768;
  }
  _rowIndex[0] = 0;
  _byteLength = 0;
  _numFrames = 0;

  if (_config->_dataLine > 0) {
    QFile file(_filename);
    if (!file.open(IO_ReadOnly)) {
      return false;
    }
    int left = _config->_dataLine;
    int didRead = 0;
    QString ignore;
#if QT_VERSION >= 0x030200
    ignore.reserve(1001);
#endif
    while (left > 0) {
      int thisRead = file.readLine(ignore, 1000);
      if (thisRead <= 0 || file.atEnd()) {
        return false;
      }
      didRead += thisRead;
      --left;
    }
    _rowIndex[0] = didRead;
  }

  return true;
}


#define MAXBUFREADLEN 32768
KstObject::UpdateType AsciiSource::update(int u) {
  if (KstObject::checkUpdateCounter(u)) {
    return lastUpdateResult();
  }

  if (!_haveHeader) {
    _haveHeader = initRowIndex();
    if (!_haveHeader) {
      return setLastUpdateResult(KstObject::NO_CHANGE);
    }
    // Re-update the field list since we have one now
    _fields = fieldListFor(_filename, _config);
    _fieldListComplete = _fields.count() > 1;
  }

  bool forceUpdate = false;
  QFile file(_filename);
  if (file.exists()) {
    if (uint(_byteLength) != file.size() || !_valid) {
      forceUpdate = true;
    }
    _byteLength = file.size();
  } else {
    _valid = false;
    return setLastUpdateResult(KstObject::NO_CHANGE);
  }

  if (!file.open(IO_ReadOnly)) {
    // quietly fail - no data to be had here
    _valid = false;
    return setLastUpdateResult(KstObject::NO_CHANGE);
  }

  _valid = true;

  int bufstart, bufread;
  bool new_data = false;
  char tmpbuf[MAXBUFREADLEN];
  do {
    /* Read the tmpbuffer, starting at row_index[_numFrames] */
    if (_byteLength - _rowIndex[_numFrames] > MAXBUFREADLEN) {
      bufread = MAXBUFREADLEN;
    } else {
      bufread = _byteLength - _rowIndex[_numFrames];
    }

    bufstart = _rowIndex[_numFrames];
    file.at(bufstart); // expensive?
    file.readBlock(tmpbuf, bufread);

    bool is_comment = false, has_dat = false;
    for (int i = 0; i < bufread; i++) {
      // Statistically numbers should be more common than comments, so let's
      // check for them first
      // if isdigit() generates a function call, it would be better to write
      // this inline, but I'm not convinced that compilers are that stupid
      // yet.  Profiling demo.kst shows this code path is hit > 10^5 times.
      if (isdigit(tmpbuf[i])) {
        if (!is_comment) {
          has_dat = true;
        }
      } else if (tmpbuf[i] == '\n' || tmpbuf[i] == '\r') {
        if (has_dat) {
          _numFrames++;
          if (_numFrames >= _numLinesAlloc) {
            _numLinesAlloc += 32768;
            _rowIndex = (int *)realloc(_rowIndex, _numLinesAlloc*sizeof(int));
          }
          new_data = true;
        }
        _rowIndex[_numFrames] = bufstart + i + 1;
        has_dat = is_comment = false;
      } else if (_config->_delimiters.contains(tmpbuf[i])) {
        is_comment = true;
      }
    }
  } while (bufread == MAXBUFREADLEN);

  file.close();

  updateNumFramesScalar();
  return setLastUpdateResult(forceUpdate ? KstObject::UPDATE : (new_data ? KstObject::UPDATE : KstObject::NO_CHANGE));
}


bool AsciiSource::fieldListIsComplete() const {
  return _fieldListComplete;
}


int AsciiSource::readField(double *v, const QString& field, int s, int n) {
  if (n < 0) {
    n = 1; /* n < 0 means read one sample, not frame - irrelevent here */
  }

  if (field == "INDEX") {
    for (int i = 0; i < n; i++) {
      v[i] = double(s + i);
    }
    return n;
  }

  QStringList fieldList = this->fieldList();
  uint col = 0;
  for (QStringList::ConstIterator i = fieldList.begin(); i != fieldList.end(); ++i) {
    if (*i == field) {
      break;
    }
    ++col;
  }

  if (col + 1 > fieldList.count()) {
    if (_fieldListComplete) {
      return 0;
    }
    bool ok = false;
    col = field.toInt(&ok);
    if (!ok) {
      return 0;
    }
  }

  int bufstart = _rowIndex[s];
  int bufread = _rowIndex[s + n] - bufstart;

  if (bufread <= 0) {
    return 0;
  }

  QFile file(_filename);
  if (!file.open(IO_ReadOnly)) {
    _valid = false;
    return 0;
  }

  if (_tmpBufSize < unsigned(bufread)) {
    _tmpBuf = static_cast<char*>(realloc(_tmpBuf, _tmpBufSize = bufread));
    if (!_tmpBuf) {
      return -1;
    }
  }

  file.at(bufstart);
  file.readBlock(_tmpBuf, bufread);

  if (_config->_columnType == AsciiSource::Config::Fixed) {
    for (int i = 0; i < n; i++, s++) {
      // Read appropriate column and convert to double
      v[i] = atof(_tmpBuf + _rowIndex[i] - _rowIndex[0] + _config->_columnWidth * (col - 1));
    }
  } else if (_config->_columnType == AsciiSource::Config::Custom) {
    for (int i = 0; i < n; i++, s++) {
      bool incol = false;
      uint i_col = 0;
      v[i] = 0.0;
      for (int ch = _rowIndex[s] - bufstart; ch < bufread; ++ch) {
        if (_config->_columnDelimiter.contains(_tmpBuf[ch])) {
          incol = false;
        } else if (_tmpBuf[ch] == '\n' || _tmpBuf[ch] == '\r') {
          break;
        } else if (_config->_delimiters.contains(_tmpBuf[ch])) {
          break;
        } else {
          if (!incol) {
            incol = true;
            i_col++;
            if (i_col == col) {
              v[i] = atof(_tmpBuf + ch);
              break;
            }
          }
        }
      }
    }
  } else {
    for (int i = 0; i < n; i++, s++) {
      bool incol = false;
      uint i_col = 0;
      v[i] = 0.0;
      for (int ch = _rowIndex[s] - bufstart; ch < bufread; ++ch) {
        if (isspace(_tmpBuf[ch])) {
          if (_tmpBuf[ch] == '\n' || _tmpBuf[ch] == '\r') {
            break;
          } else {
            incol = false;
          }
        } else if (_config->_delimiters.contains(_tmpBuf[ch])) {
          break;
        } else {
          if (!incol) {
            incol = true;
            i_col++;
            if (i_col == col) {
              v[i] = atof(_tmpBuf + ch);
              break;
            }
          }
        }
      }
    }
  }

  file.close();

  return n;
}


bool AsciiSource::isValidField(const QString& field) const {
  return fieldList().contains(field);
}


int AsciiSource::samplesPerFrame(const QString &field) {
  Q_UNUSED(field)
  return 1;
}


int AsciiSource::frameCount(const QString& field) const {
  Q_UNUSED(field)
  return _numFrames;
}


QString AsciiSource::fileType() const {
  return "ASCII";
}


bool AsciiSource::isEmpty() const {
  return _numFrames < 1;
}


QStringList AsciiSource::fieldListFor(const QString& filename, AsciiSource::Config *cfg) {
  QStringList rc;
  QFile file(filename);

  if (!file.open(IO_ReadOnly)) {
    return rc;
  }

  rc += "INDEX";

  if (cfg->_readFields) {
    int l = cfg->_fieldsLine;
    QString line;
    while (!file.atEnd()) {
      int r = file.readLine(line, 1000);
      if (l-- == 0) {
        if (r >= 0) {
          if (cfg->_columnType == AsciiSource::Config::Custom && !cfg->_columnDelimiter.isEmpty()) {
            rc += QStringList::split(QRegExp(QString("[%1]").arg(QRegExp::escape(cfg->_columnDelimiter))), line.stripWhiteSpace(), false);
          } else if (cfg->_columnType == AsciiSource::Config::Fixed) {
            int cnt = line.length() / cfg->_columnWidth;
            for (int i = 0; i < cnt; ++i) {
              QString sub = line.mid(i * cfg->_columnWidth).left(cfg->_columnWidth);
              rc += sub.stripWhiteSpace();
            }
          } else {
            rc += QStringList::split(QRegExp("[\\s]"), line.stripWhiteSpace(), false);
          }
        }
        break;
      }
    }
    return rc;
  }

  QRegExp re;
  if (cfg->_columnType == AsciiSource::Config::Custom && !cfg->_columnDelimiter.isEmpty()) {
    re = QString("^[%1]*[%2].*").arg(QRegExp::escape(cfg->_columnDelimiter)).arg(cfg->_delimiters);
  } else {
    re = QString("^\\s*[%1].*").arg(cfg->_delimiters);
  }

  bool done = false;
  QString line;
  int skip = cfg->_dataLine;
  while (!file.atEnd() && !done) {
      int r = file.readLine(line, 1000);
      if (skip > 0) {
        --skip;
        if (r < 0) {
          return rc;
        }
        continue;
      }
      if (r > 1 && !re.exactMatch(line)) {
        done = true;
      } else if (r < 0) {
        return rc;
      }
  }

  file.close();

  int cnt;
  line = line.stripWhiteSpace();
  if (cfg->_columnType == AsciiSource::Config::Custom && !cfg->_columnDelimiter.isEmpty()) {
    cnt = QStringList::split(QRegExp(QString("[%1]").arg(QRegExp::escape(cfg->_columnDelimiter))), line, false).count();
  } else if (cfg->_columnType == AsciiSource::Config::Fixed) {
    cnt = line.length() / cfg->_columnWidth;
  } else {
    cnt = QStringList::split(QRegExp("\\s"), line, false).count();
  }
  for (int i = 1; i <= cnt; ++i) {
    rc += QString::number(i);
  }

  return rc;
}


QStringList AsciiSource::fieldList() const {
  if (_fields.isEmpty()) {
    _fields = fieldListFor(_filename, _config);
    _fieldListComplete = _fields.count() > 1;
  }

  return _fields;
}


void AsciiSource::save(QTextStream &ts, const QString& indent) {
  KstDataSource::save(ts, indent);
  _config->save(ts, indent);
}


bool AsciiSource::supportsTimeConversions() const {
  return false; //fieldList().contains(_config->_indexVector) && _config->_indexInterpretation != AsciiSource::Config::Unknown && _config->_indexInterpretation != AsciiSource::Config::INDEX;
}


int AsciiSource::sampleForTime(int seconds, bool *ok) {
  switch (_config->_indexInterpretation) {
    case AsciiSource::Config::Seconds:
      // FIXME: make sure "seconds" exists in _indexVector
      if (ok) {
        *ok = true;
      }
      return seconds;
    case AsciiSource::Config::CTime:
      // FIXME: make sure "seconds" exists in _indexVector (different than above?)
      if (ok) {
        *ok = true;
      }
      return seconds;
    default:
      return KstDataSource::sampleForTime(seconds, ok);
  }
}


int AsciiSource::sampleForTime(const QDateTime& time, bool *ok) {
  switch (_config->_indexInterpretation) {
    case AsciiSource::Config::Seconds:
      // FIXME: make sure "time" exists in _indexVector
      if (ok) {
        *ok = true;
      }
      return time.toTime_t();
    case AsciiSource::Config::CTime:
      // FIXME: make sure "time" exists in _indexVector (different than above?)
      if (ok) {
        *ok = true;
      }
      return time.toTime_t();
    default:
      return KstDataSource::sampleForTime(time, ok);
  }
}


class ConfigWidgetAscii : public KstDataSourceConfigWidget {
  public:
    ConfigWidgetAscii() : KstDataSourceConfigWidget() {
      QGridLayout *layout = new QGridLayout(this, 1, 1);
      _ac = new AsciiConfig(this);
      layout->addWidget(_ac, 0, 0);
      layout->activate();
    }

    virtual ~ConfigWidgetAscii() {}

    virtual void setConfig(KConfig *cfg) {
      KstDataSourceConfigWidget::setConfig(cfg);
    }

    virtual void load() {
      _cfg->setGroup("ASCII General");
      _ac->_delimiters->setText(_cfg->readEntry("Comment Delimiters", DEFAULT_DELIMITERS));
      _ac->_fileNamePattern->setText(_cfg->readEntry("Filename Pattern", QString::null));
      _ac->_columnDelimiter->setText(_cfg->readEntry("Column Delimiter", QString::null));
      _ac->_columnWidth->setValue(_cfg->readNumEntry("Column Width", DEFAULT_COLUMN_WIDTH));
      _ac->_startLine->setValue(_cfg->readNumEntry("Data Start", 0));
      _ac->_readFields->setChecked(_cfg->readBoolEntry("Read Fields", false));
      _ac->_fieldsLine->setValue(_cfg->readNumEntry("Fields Line", 0));
      AsciiSource::Config::ColumnType ct = (AsciiSource::Config::ColumnType)_cfg->readNumEntry("Column Type", 0);
      if (ct == AsciiSource::Config::Fixed) {
        _ac->_fixed->setChecked(true);
      } else if (ct == AsciiSource::Config::Custom) {
        _ac->_custom->setChecked(true);
      } else {
        _ac->_whitespace->setChecked(true);
      }
      bool hasInstance = _instance != 0L;
      _ac->_indexVector->clear();
      if (hasInstance) {
        _ac->_indexVector->insertStringList(_instance->fieldList());
        KstSharedPtr<AsciiSource> src = kst_cast<AsciiSource>(_instance);
        assert(src);
        _ac->_indexType->setCurrentItem(src->_config->_indexInterpretation - 1);
        if (_instance->fieldList().contains(src->_config->_indexVector)) {
          _ac->_indexVector->setCurrentText(src->_config->_indexVector);
        }
        
        _cfg->setGroup(src->fileName());
        _ac->_delimiters->setText(_cfg->readEntry("Comment Delimiters", _ac->_delimiters->text()));
        _ac->_columnDelimiter->setText(_cfg->readEntry("Column Delimiter", _ac->_columnDelimiter->text()));
        _ac->_columnWidth->setValue(_cfg->readNumEntry("Column Width", _ac->_columnWidth->value()));
        _ac->_startLine->setValue(_cfg->readNumEntry("Data Start", _ac->_startLine->value()));
        _ac->_readFields->setChecked(_cfg->readBoolEntry("Read Fields", _ac->_readFields->isChecked()));
        _ac->_fieldsLine->setValue(_cfg->readNumEntry("Fields Line", _ac->_fieldsLine->value()));
        ct = (AsciiSource::Config::ColumnType)_cfg->readNumEntry("Column Type", ct);
        if (ct == AsciiSource::Config::Fixed) {
          _ac->_fixed->setChecked(true);
        } else if (ct == AsciiSource::Config::Custom) {
          _ac->_custom->setChecked(true);
        } else {
          _ac->_whitespace->setChecked(true);
        }
      } else {
        _ac->_indexVector->insertItem("INDEX");
        int x = _cfg->readNumEntry("Default INDEX Interpretation", AsciiSource::Config::INDEX);
        if (x > 0 && x <= _ac->_indexType->count()) {
          _ac->_indexType->setCurrentItem(x - 1);
        } else {
          _ac->_indexType->setCurrentItem(0);
        }
      }
      _ac->_indexVector->setEnabled(hasInstance);
    }

    virtual void save() {
      assert(_cfg);
      _cfg->setGroup("ASCII General");
      _cfg->writeEntry("Filename Pattern", _ac->_fileNamePattern->text());
      // If we have an instance, save settings for that instance only
      KstSharedPtr<AsciiSource> src = kst_cast<AsciiSource>(_instance);
      if (src) {
        _cfg->setGroup(src->fileName());
      }
      _cfg->writeEntry("Default INDEX Interpretation", 1 + _ac->_indexType->currentItem());
      _cfg->writeEntry("Comment Delimiters", _ac->_delimiters->text());
      AsciiSource::Config::ColumnType ct = AsciiSource::Config::Whitespace;
      if (_ac->_fixed->isChecked()) {
        ct = AsciiSource::Config::Fixed;
      } else if (_ac->_custom->isChecked()) {
        ct = AsciiSource::Config::Custom;
      }
      _cfg->writeEntry("Column Type", (int)ct);
      _cfg->writeEntry("Column Delimiter", _ac->_columnDelimiter->text());
      _cfg->writeEntry("Column Width", _ac->_columnWidth->value());
      _cfg->writeEntry("Data Start", _ac->_startLine->value());
      _cfg->writeEntry("Read Fields", _ac->_readFields->isChecked());
      _cfg->writeEntry("Fields Line", _ac->_fieldsLine->value());

      // Update the instance from our new settings
      if (src && src->reusable()) {
        src->_config->read(_cfg, src->fileName());
      }
    }

    AsciiConfig *_ac;
};


extern "C" {
KstDataSource *create_ascii(KConfig *cfg, const QString& filename, const QString& type) {
  return new AsciiSource(cfg, filename, type);
}

KstDataSource *load_ascii(KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e) {
  return new AsciiSource(cfg, filename, type, e);
}

QStringList provides_ascii() {
  QStringList rc;
  rc += "ASCII";
  return rc;
}

int understands_ascii(KConfig *cfg, const QString& filename) {
  AsciiSource::Config config;
  config.read(cfg, filename);

  if (!config._fileNamePattern.isEmpty()) {
    QRegExp filenamePattern(config._fileNamePattern);
    filenamePattern.setWildcard(true);
    if (filenamePattern.exactMatch(filename)) {
      return 100;
    }
  }

  if (!QFile::exists(filename) || QFileInfo(filename).isDir()) {
    return 0;
  }

  QFile f(filename);
  if (f.open(IO_ReadOnly)) {
    QString s;
    Q_LONG rc = 0;
    bool done = false;

    QRegExp commentRE, dataRE;
    if (config._columnType == AsciiSource::Config::Custom && !config._columnDelimiter.isEmpty()) {
      commentRE = QString("^[%1]*[%2].*").arg(QRegExp::escape(config._columnDelimiter)).arg(config._delimiters);
      dataRE = QString("^[%1]*(([Nn][Aa][Nn]|(\\-\\+)?[Ii][Nn][Ff]|[0-9\\+\\-\\.eE]+)[\\s]*)+").arg(QRegExp::escape(config._columnDelimiter));
    } else {
      commentRE = QString("^\\s*[%1].*").arg(config._delimiters);
      dataRE = QString("^[\\s]*(([Nn][Aa][Nn]|(\\-\\+)?[Ii][Nn][Ff]|[0-9\\+\\-\\.eE]+)[\\s]*)+");
    }

    int skip = config._dataLine;

    while (!done) {
      rc = f.readLine(s, 1000);
      if (skip > 0) {
        --skip;
        if (rc <= 0) {
          done = true;
        }
        continue;
      }
      if (rc <= 0) {
        done = true;
      } else if (rc == 1) {
        // empty line; do nothing
      } else if (commentRE.exactMatch(s)) {
        // comment; do nothing
      } else if (dataRE.exactMatch(s)) {
        // a number - this may be an ascii file - assume that it is
        // This line checks for an indirect file and gives that a chance too.
        // Indirect files look like ascii files.
        return QFile::exists(s.stripWhiteSpace()) ? 49 : 75;
      } else {
        return 20;
      }
    }
  } else {
    return 0;
  }

  return 1; // still might be ascii - ex: header with no data yet.
}


QStringList fieldList_ascii(KConfig *cfg, const QString& filename, const QString& type, QString *typeSuggestion, bool *complete) {
  if ((!type.isEmpty() && !provides_ascii().contains(type)) ||
      0 == understands_ascii(cfg, filename)) {
    if (complete) {
      *complete = false;
    }
    return QStringList();
  }

  if (typeSuggestion) {
    *typeSuggestion = "ASCII";
  }

  AsciiSource::Config config;
  config.read(cfg, filename);
  QStringList rc = AsciiSource::fieldListFor(filename, &config);

  if (complete) {
    *complete = rc.count() > 1;
  }

  return rc;
}


QWidget *widget_ascii(const QString& filename) {
  Q_UNUSED(filename)
  return new ConfigWidgetAscii;
}

}

// vim: ts=2 sw=2 et
