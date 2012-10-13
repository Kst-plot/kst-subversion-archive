/***************************************************************************
                      ASCII file data source
                             -------------------
    begin                : Fri Oct 17 2003
    copyright            : (C) 2003 The University of Toronto
     email                : netterfield@astro.utoronto.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "asciisource.h"
#include "asciisourceconfig.h"
#include "asciidatainterfaces.h"

#include "curve.h"
#include "colorsequence.h"
#include "objectstore.h"

#include "math_kst.h"
#include "kst_inf.h"
#include "kst_i18n.h"
#include "kst_atof.h"
#include "measuretime.h"

#include <QFile>
#include <QMessageBox>
        
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <iostream>


// Enable QASSERT in QVarLengthArray  when using [] on data
#if 0
  #define constData constArray
#else
  #define constData constPointer // loads faster in debug mode
#endif
    
    
// simulate out of memory scenario
//#define KST_TEST_OOM

using namespace Kst;


//-------------------------------------------------------------------------------------------
static const QString asciiTypeString = I18N_NOOP("ASCII file");


//-------------------------------------------------------------------------------------------
const QString AsciiSource::asciiTypeKey()
{
  return asciiTypeString;
}


//-------------------------------------------------------------------------------------------
AsciiSource::AsciiSource(Kst::ObjectStore *store, QSettings *cfg, const QString& filename, const QString& type, const QDomElement& e) :
  Kst::DataSource(store, cfg, filename, type),
  r(_config),
  _fileBuffer(new AsciiDataReader::FileBuffer),
  _rowIndex(),
  is(new DataInterfaceAsciiString(*this)),
  iv(new DataInterfaceAsciiVector(*this))
{
#ifdef KST_TEST_OOM
  // eat the giga bytes
	while (qMalloc(1024*1024*1024) != 0) {}
#endif

  setInterface(is);
  setInterface(iv);

  reset();

  // TODO only works for local files
  setUpdateType(File);

  _source = asciiTypeString;
  if (!type.isEmpty() && type != asciiTypeString) {
    return;
  }

  _config.readGroup(*cfg, filename);
  if (!e.isNull()) {
    _config.load(e);
  }

  _valid = true;
  registerChange();
  internalDataSourceUpdate(false);
}


//-------------------------------------------------------------------------------------------
AsciiSource::~AsciiSource() 
{
  delete _fileBuffer;
}


//-------------------------------------------------------------------------------------------
void AsciiSource::reset() 
{
  // forget about cached data
  _fileBuffer->clearFileBuffer();
  _rowIndex.clear();

  _valid = false;
  _byteLength = 0;
  _numFrames = 0;
  _haveHeader = false;
  _fieldListComplete = false;

  _fieldList.clear();
  _scalarList.clear();
  _strings.clear();

  Object::reset();

  _strings = fileMetas();
}


//-------------------------------------------------------------------------------------------
bool AsciiSource::openFile(QFile &file) 
{
  // Don't use 'QIODevice::Text'!
  // Because CR LF line ending breaks row offset calculation
  return file.open(QIODevice::ReadOnly);
}


//-------------------------------------------------------------------------------------------
bool AsciiSource::openValidFile(QFile &file) 
{
  _valid = openFile(file);
  return _valid;
}


//-------------------------------------------------------------------------------------------
bool AsciiSource::initRowIndex() 
{
  // capacity is at least the pre-allocated memory
  _rowIndex.resize(_rowIndex.capacity());

  _rowIndex[0] = 0;
  _byteLength = 0;
  _numFrames = 0;

  if (_config._dataLine > 0) {
    QFile file(_filename);
    if (!openValidFile(file)) {
      return false;
    }
    int header_row = 0;
    int left = _config._dataLine;
    int didRead = 0;
    while (left > 0) {
      QByteArray line = file.readLine();
      if (line.isEmpty() || file.atEnd()) {
        return false;
      }
      didRead += line.size();
      --left;
      if (header_row != _config._fieldsLine && header_row != _config._unitsLine) {
        _strings[QString("Header %1").arg(header_row, 2, 10, QChar('0'))] = QString::fromAscii(line).trimmed();
      }
      header_row++;
    }
    _rowIndex[0] = didRead;
  }

  return true;
}




//-------------------------------------------------------------------------------------------
Kst::Object::UpdateType AsciiSource::internalDataSourceUpdate()
{
  return internalDataSourceUpdate(true);
}


//-------------------------------------------------------------------------------------------
Kst::Object::UpdateType AsciiSource::internalDataSourceUpdate(bool read_completely)
{
  MeasureTime t("AsciiSource::internalDataSourceUpdate: " + _filename);

  // forget about cached data
  _fileBuffer->clearFileBuffer();

  if (!_haveHeader) {
    _haveHeader = initRowIndex();
    if (!_haveHeader) {
      return NoChange;
    }
    // Re-update the field list since we have one now
    _fieldList = fieldListFor(_filename, &_config);
    QStringList units;
    if (_config._readUnits) {
      units += unitListFor(_filename, &_config);
      for (int index = 0; index < _fieldList.size(); ++index) {
        if (index >= units.size()) {
          break; // Missing units => the user's fault, but at least don't crash
        }
        _fieldUnits[_fieldList[index]] = units[index];
      }
    }
    _fieldListComplete = _fieldList.count() > 1;

    // Re-update the scalar list since we have one now
    _scalarList = scalarListFor(_filename, &_config);
  }

  QFile file(_filename);
  if (!openValidFile(file)) {
    // Qt: If the device is closed, the size returned will not reflect the actual size of the device.
    return NoChange;
  }

  AsciiDataReader::LineEndingType lineending = r.detectLineEndingType(file);

  bool new_data = false;
  bool force_update = true;
  if (_byteLength == file.size()) {
    force_update = false;
  }
  _byteLength = file.size();

  AsciiDataReader::FileBuffer buf;
  buf._bufferedS = _rowIndex[_numFrames];
  buf._bufferedN = 0;
  do {
    // Read the tmpbuffer, starting at row_index[_numFrames]
    buf.clear();
    buf.resize(buf.capacity());

    //bufstart += bufread;
    buf._bufferedS = _rowIndex[_numFrames]; // always read from the start of a line
    buf._bufferedN = r.readFromFile(file, buf, buf._bufferedS, _byteLength - buf._bufferedS, AsciiDataReader::FileBuffer::Prealloc - 1);

    if (_config._delimiters.value().size() == 0) {
      const AsciiDataReader::NoDelimiter comment_del;
      if (lineending.isLF()) {
        new_data = findDataRows(buf.constData(), buf._bufferedS, buf._bufferedN, AsciiDataReader::IsLineBreakLF(lineending), comment_del);
      } else {
        new_data = findDataRows(buf.constData(), buf._bufferedS, buf._bufferedN, AsciiDataReader::IsLineBreakCR(lineending), comment_del);
      }
    } else if (_config._delimiters.value().size() == 1) {
      const AsciiDataReader::IsCharacter comment_del(_config._delimiters.value()[0].toLatin1());
      if (lineending.isLF()) {
        new_data = findDataRows(buf.constData(), buf._bufferedS, buf._bufferedN, AsciiDataReader::IsLineBreakLF(lineending), comment_del);
      } else {
        new_data = findDataRows(buf.constData(), buf._bufferedS, buf._bufferedN, AsciiDataReader::IsLineBreakCR(lineending), comment_del);
      }
    } else if (_config._delimiters.value().size() > 1) {
      const AsciiDataReader::IsInString comment_del(_config._delimiters.value());
      if (lineending.isLF()) {
        new_data = findDataRows(buf.constData(), buf._bufferedS, buf._bufferedS, AsciiDataReader::IsLineBreakLF(lineending), comment_del);
      } else {
        new_data = findDataRows(buf.constData(), buf._bufferedS, buf._bufferedN, AsciiDataReader::IsLineBreakCR(lineending), comment_del);
      }
    }

  } while (buf._bufferedN == AsciiDataReader::FileBuffer::Prealloc - 1  && read_completely);

  _rowIndex.resize(_numFrames+1);

  return (!new_data && !force_update ? NoChange : Updated);
}

template<class Buffer, typename IsLineBreak, typename CommentDelimiter>
bool AsciiSource::findDataRows(const Buffer& buffer, int bufstart, int bufread, const IsLineBreak& isLineBreak, const CommentDelimiter& comment_del)
{
  const AsciiDataReader::IsWhiteSpace isWhiteSpace;
  
  bool new_data = false;
  bool row_has_data = false;
  bool is_comment = false;
  const int row_offset = bufstart + isLineBreak.size;
  int row_start = bufstart;

  for (int i = 0; i < bufread; i++) {
    if (comment_del(buffer[i])) {
      is_comment = true;
    } else if (isLineBreak(buffer[i])) {
      if (row_has_data) {
        _rowIndex[_numFrames] = row_start;
        ++_numFrames;
        if (_numFrames >= _rowIndex.size()) {
          _rowIndex.resize(_rowIndex.size() + AsciiDataReader::FileBuffer::Prealloc - 1);
        }
        new_data = true;
        row_start = row_offset+i;
      } else if (is_comment) {
        row_start = row_offset+i;
      }
      row_has_data = false;
      is_comment = false;
    } else if (!row_has_data && !isWhiteSpace(buffer[i]) && !is_comment) {
      row_has_data = true;
    }
  }
  _rowIndex[_numFrames] = row_start;
  return new_data;
}

//-------------------------------------------------------------------------------------------
int AsciiSource::columnOfField(const QString& field) const
{
  if (_fieldList.contains(field)) {
    return _fieldList.indexOf(field);
  } 

  if (_fieldListComplete) {
    return -1;
  }

  bool ok = false;
  int col = field.toInt(&ok);
  if (ok) {
    return col;
  }

  return -1;
}




//-------------------------------------------------------------------------------------------
int AsciiSource::readField(double *v, const QString& field, int s, int n) 
{
  bool re_alloc;
  int n_read = readField(v, field, s, n, re_alloc);
  if (re_alloc) {
    // file is now buffered in memory
    return n_read;
  }

  // reading whole file into memory failed

  // find a smaller allocatable size
  _fileBuffer->clearFileBuffer();
  int realloc_size = n / 4;
  while (!_fileBuffer->resize(realloc_size) && realloc_size > 0) {
      realloc_size /= 2;
  }
  _fileBuffer->clearFileBuffer();
  if (realloc_size == 0) {
    QMessageBox::warning(0, "Error while reading ascii file", "File could not be read because not enough memory is available.");
    return 0;      
  }

  // read in 
  int start = s;
  n_read = 0;
  while (n_read < n) {
    _fileBuffer->clearFileBuffer();
    int to_read = n_read + realloc_size < n ? realloc_size : n - n_read;
    n_read += readField(v + start, field, n_read, to_read, re_alloc);
    if (!re_alloc) {
      _fileBuffer->clearFileBuffer();
      QMessageBox::warning(0, "Error while reading ascii file", "The file was only read partially not enough memory is available.");
      return n_read; 
    }
    start += to_read;
  }
  // don't buffer partial files
  _fileBuffer->clearFileBuffer();
  return n_read;
}


//-------------------------------------------------------------------------------------------
int AsciiSource::readField(double *v, const QString& field, int s, int n, bool& re_alloc) 
{
  re_alloc = true;
  if (n < 0) {
    n = 1; /* n < 0 means read one sample, not frame - irrelevent here */
  }

  if (field == "INDEX") {
    for (int i = 0; i < n; i++) {
      v[i] = double(s + i);
    }
    return n;
  }

  int col = columnOfField(field);
  if (col == -1) {
    return 0;
  }

  int bufstart = _rowIndex[s];
  int bufread = _rowIndex[s + n] - bufstart;
  if (bufread <= 0) {
    return 0;
  }

  if ((s != _fileBuffer->_bufferedS) || (n != _fileBuffer->_bufferedN)) {
    QFile file(_filename);
    if (!openValidFile(file)) {
      return 0;
    }

    r._lineending = r.detectLineEndingType(file);


    bufread = r.readFromFile(file, *_fileBuffer, bufstart, bufread);
    if (bufread == 0) {
      re_alloc = false;
      return 0;
    }
    _fileBuffer->_bufferedS = s;
    _fileBuffer->_bufferedN = n;
  }

 return r.readField(_rowIndex, _fileBuffer, col, bufstart, bufread, v, field, s, n, re_alloc);
}

int AsciiDataReader::readField(const RowIndex& _rowIndex, FileBuffer* _fileBuffer, int col, int bufstart, int bufread,
                               double *v, const QString& field, int s, int n, bool& re_alloc) 
  {

  if (_config._columnType == AsciiSourceConfig::Fixed) {
    MeasureTime t("AsciiSource::readField: same width for all columns");
    LexicalCast lexc;
    lexc.setDecimalSeparator(_config._useDot);
    // &buffer[0] points to first row at _rowIndex[0] , so if we wanna find
    // the column in row i by adding _rowIndex[i] we have to start at:
    const char* col_start = &_fileBuffer->constData()[0] - _rowIndex[0] + _config._columnWidth * (col - 1);
    for (int i = 0; i < n; ++i) {
      v[i] = lexc.toDouble(_rowIndex[i] + col_start);
    }
    return n;
  } else if (_config._columnType == AsciiSourceConfig::Custom) {
    if (_config._columnDelimiter.value().size() == 1) {
      MeasureTime t("AsciiSource::readField: 1 custom column delimiter");
      const AsciiDataReader::IsCharacter column_del(_config._columnDelimiter.value()[0].toLatin1());
      return readColumns(_rowIndex, v, _fileBuffer->constData(), bufstart, bufread, col, s, n, _lineending, column_del);
    } if (_config._columnDelimiter.value().size() > 1) {
      MeasureTime t(QString("AsciiSource::readField: %1 custom column delimiters").arg(_config._columnDelimiter.value().size()));
      const AsciiDataReader::IsInString column_del(_config._columnDelimiter.value());
      return readColumns(_rowIndex, v, _fileBuffer->constData(), bufstart, bufread, col, s, n, _lineending, column_del);
    }
  } else if (_config._columnType == AsciiSourceConfig::Whitespace) {
    MeasureTime t("AsciiSource::readField: whitespace separated columns");
    const AsciiDataReader::IsWhiteSpace column_del;
    return readColumns(_rowIndex, v, _fileBuffer->constData(), bufstart, bufread, col, s, n, _lineending, column_del);
  }

  return 0;
}


//-------------------------------------------------------------------------------------------
QString AsciiSource::fileType() const 
{
  return asciiTypeString;
}


//-------------------------------------------------------------------------------------------
bool AsciiSource::isEmpty() const 
{
  return _numFrames < 1;
}


//-------------------------------------------------------------------------------------------
QStringList AsciiSource::scalarListFor(const QString& filename, AsciiSourceConfig*) 
{
  QFile file(filename);
  if (!openFile(file)) {
    return QStringList();
  }
  return QStringList() << "FRAMES";
}


//-------------------------------------------------------------------------------------------
QStringList AsciiSource::stringListFor(const QString& filename, AsciiSourceConfig*) 
{
  QFile file(filename);
  if (!openFile(file)) {
    return QStringList();
  }
  return QStringList() << "FILE";
}


//-------------------------------------------------------------------------------------------
QStringList AsciiSource::splitHeaderLine(const QByteArray& line, AsciiSourceConfig* cfg)
{
  QStringList parts;
  const QRegExp regexColumnDelimiter(QString("[%1]").arg(QRegExp::escape(cfg->_columnDelimiter.value())));

  if (cfg->_columnType == AsciiSourceConfig::Custom && !cfg->_columnDelimiter.value().isEmpty()) {
    parts += QString(line).trimmed().split(regexColumnDelimiter, QString::SkipEmptyParts);
  } else if (cfg->_columnType == AsciiSourceConfig::Fixed) {
    int cnt = line.length() / cfg->_columnWidth;
    for (int i = 0; i < cnt; ++i) {
      QString sub = line.mid(i * cfg->_columnWidth).left(cfg->_columnWidth);
      parts += sub.trimmed();
    }
  } else {
    parts += QString(line).trimmed().split(QRegExp("\\s"), QString::SkipEmptyParts);
  }
  return parts;
}


//-------------------------------------------------------------------------------------------
QStringList AsciiSource::fieldListFor(const QString& filename, AsciiSourceConfig* cfg) 
{
  QFile file(filename);
  if (!openFile(file)) {
    return QStringList();
  }

  QStringList fields;
  fields += "INDEX";

  if (cfg->_readFields) {
    int fieldsLine = cfg->_fieldsLine;
    int currentLine = 0; // Explicit line counter, to make the code easier to understand
    while (currentLine < cfg->_dataLine) {
      const QByteArray line = file.readLine();
      int r = line.size();
      if (currentLine == fieldsLine && r >= 0) {
        fields += AsciiSource::splitHeaderLine(line, cfg);
        break;
      }
      currentLine++;
    }
    QStringList trimmed;
    foreach(const QString& str, fields) {
      trimmed << str.trimmed();
    }
    return trimmed;
  }

  
  QRegExp regex;
  if (cfg->_columnType == AsciiSourceConfig::Custom && !cfg->_columnDelimiter.value().isEmpty()) {
    regex.setPattern(QString("^[%1]*[%2].*").arg(QRegExp::escape(cfg->_columnDelimiter.value())).arg(cfg->_delimiters));
  } else {
    regex.setPattern(QString("^\\s*[%1].*").arg(cfg->_delimiters));
  }

  bool done = false;
  int skip = cfg->_dataLine;
  //FIXME This is a hack which should eventually be fixed by specifying
  // the starting frame of the data when calling KstDataSource::fieldListForSource
  // and KstDataSource::fieldList.  If the skip value is not specified, then
  // we scan a few lines and take the maximum number of fields that we find.
  int maxcnt;
  if (skip > 0) {
    maxcnt = -1;
  } else {
    maxcnt = 0;
  }
  int cnt;
  int nextscan = 0;
  int curscan = 0;
  while (!file.atEnd() && !done && (nextscan < 200)) {
    QByteArray line = file.readLine();
    int r = line.size();
    if (skip > 0) { //keep skipping until desired line
      --skip;
      if (r < 0) {
        return fields;
      }
      continue;
    }
    if (maxcnt >= 0) { //original skip value == 0, so scan some lines
      if (curscan >= nextscan) {
        if (r > 1 && !regex.exactMatch(line)) {
          cnt = splitHeaderLine(line, cfg).count();         
          if (cnt > maxcnt) {
            maxcnt = cnt;
          }
        } else if (r < 0) {
          return fields;
        }
        nextscan += nextscan + 1;
      }
      curscan++;
      continue;
    }
    if (r > 1 && !regex.exactMatch(line)) { //at desired line, find count
      maxcnt = splitHeaderLine(line, cfg).count(); 
      done = true;
    } else if (r < 0) {
      return fields;
    }
  }

  for (int i = 1; i <= maxcnt; ++i) {
    fields += i18n("Column %1").arg(i).trimmed();
  }

  return fields;
}

QStringList AsciiSource::unitListFor(const QString& filename, AsciiSourceConfig* cfg)
{
  QFile file(filename);
  if (!openFile(file)) {
    return QStringList();
  }

  QStringList units;
  units += ""; // To go with INDEX

  int unitsLine = cfg->_unitsLine;
  int currentLine = 0;
  while (currentLine < cfg->_dataLine) {
    const QByteArray line = file.readLine();
    int r = line.size();
    if (currentLine == unitsLine && r >= 0) {
      units += AsciiSource::splitHeaderLine(line, cfg);
      break;
    }
    currentLine++;
  }
  QStringList trimmed;
  foreach(const QString& str, units) {
    trimmed << str.trimmed();
  }
  return trimmed;
}


//-------------------------------------------------------------------------------------------
void AsciiSource::save(QXmlStreamWriter &s) 
{
  Kst::DataSource::save(s);
  _config.save(s);
}


//-------------------------------------------------------------------------------------------
void AsciiSource::parseProperties(QXmlStreamAttributes &properties) 
{
  _config.parseProperties(properties);
  reset();
  internalDataSourceUpdate();
}


//-------------------------------------------------------------------------------------------
bool AsciiSource::supportsTimeConversions() const 
{
  return false; //fieldList().contains(_config._indexVector) && _config._indexInterpretation != AsciiSourceConfig::Unknown && _config._indexInterpretation != AsciiSourceConfig::INDEX;
}


//-------------------------------------------------------------------------------------------
int AsciiSource::sampleForTime(double ms, bool *ok) 
{
  switch (_config._indexInterpretation) {
    case AsciiSourceConfig::Seconds:
      // FIXME: make sure "seconds" exists in _indexVector
      if (ok) {
        *ok = true;
      }
      return 0;
    case AsciiSourceConfig::CTime:
      // FIXME: make sure "seconds" exists in _indexVector (different than above?)
      if (ok) {
        *ok = true;
      }
      return 0;
    default:
      return Kst::DataSource::sampleForTime(ms, ok);
  }
}


//-------------------------------------------------------------------------------------------
const QString& AsciiSource::typeString() const 
{
  return asciiTypeString;
}


//-------------------------------------------------------------------------------------------
int AsciiSource::sampleForTime(const QDateTime& time, bool *ok) 
{
  switch (_config._indexInterpretation) {
    case AsciiSourceConfig::Seconds:
      // FIXME: make sure "time" exists in _indexVector
      if (ok) {
        *ok = true;
      }
      return time.toTime_t();
    case AsciiSourceConfig::CTime:
      // FIXME: make sure "time" exists in _indexVector (different than above?)
      if (ok) {
        *ok = true;
      }
      return time.toTime_t();
    default:
      return Kst::DataSource::sampleForTime(time, ok);
  }
}



Kst::ObjectList<Kst::Object> AsciiSource::autoCurves(ObjectStore& objectStore)
{
  // here we could do more sophisticated stuff when generating a list of curves
  return ObjectList<Kst::Object>();
}



// vim: ts=2 sw=2 et
