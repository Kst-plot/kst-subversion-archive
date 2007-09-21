/***************************************************************************
                     scuba.cpp  -  SCUBA file data source
                             -------------------
    begin                : Tue Jan 22 2007
    copyright            : (C) 2007 The University of British Columbia
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


#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qstylesheet.h>

#include <kcombobox.h>
#include <ksdebug.h>

#include <kstmath.h>
#include "kststring.h"
#include "scuba.h"
#include "scubaconfig.h"

#define BEGIN_HEADRER_1 "ac"
#define END_HEADER_1 "end_status\n"

#define BEGIN_HEADER_2 "<HEADER>"
#define END_HEADER_2 "</HEADER>"

#define MAXBUFREADLEN 32768
#define FRAMEINDEXBUFFERINCREMENT 32768

#define DEFAULT_COLUMN_WIDTH 16

#define RAW_DATA_BUFFER_SIZE  8192

#define COLUMNS_PER_READOUTCARD 8
#define COLUMNS_PER_TEXTFORMAT  8

static char* housekeepingFields[] = {
    "Status",                 // value  0 of housekeeping block
    "Sequence Number",        // value  1 of housekeeping block
    "Sync Number",            // value  2 of housekeeping block
    "DV Pulse Number",        // value  3 of housekeeping block

    "Comm Error FPGA Temp",   // value  4 of housekeeping block
    "AC FPGA Temp",           // value  5 of housekeeping block
    "BC1 FPGA Temp",          // value  6 of housekeeping block
    "BC2 FPGA Temp",          // value  7 of housekeeping block
    "BC3 FPGA Temp",          // value  8 of housekeeping block
    "RC1 FPGA Temp",          // value  9 of housekeeping block
    "RC2 FPGA Temp",          // value 10 of housekeeping block
    "RC3 FPGA Temp",          // value 11 of housekeeping block
    "RC4 FPGA Temp",          // value 12 of housekeeping block
    "CC FPGA Temp",           // value 13 of housekeeping block

    "Comm Error Card Temp",   // value 14 of housekeeping block
    "AC card Temp",           // value 15 of housekeeping block
    "BC1 card Temp",          // value 16 of housekeeping block
    "BC2 card Temp",          // value 17 of housekeeping block
    "BC3 card Temp",          // value 18 of housekeeping block
    "RC1 card Temp",          // value 19 of housekeeping block
    "RC2 card Temp",          // value 20 of housekeeping block
    "RC3 card Temp",          // value 21 of housekeeping block
    "RC4 card Temp",          // value 22 of housekeeping block
    "CC card Temp",           // value 23 of housekeeping block

    "Comm Error PSUC",        // value 24 of housekeeping block
    "PSUC 1",                 // value 25 of housekeeping block
    "PSUC 2",                 // value 26 of housekeeping block
    "PSUC 3",                 // value 27 of housekeeping block
    "PSUC 4",                 // value 28 of housekeeping block
    "PSUC 5",                 // value 29 of housekeeping block
    "PSUC 6",                 // value 30 of housekeeping block
    "PSUC 7",                 // value 31 of housekeeping block

    "Comm Error Box Temp",    // value 32 of housekeeping block
    "Box Temp",               // value 33 of housekeeping block

    "",                       // value 34 of housekeeping block is currently reserved
    "",                       // value 35 of housekeeping block is currently reserved
    "",                       // value 36 of housekeeping block is currently reserved
    "",                       // value 37 of housekeeping block is currently reserved
    "",                       // value 38 of housekeeping block is currently reserved
    "",                       // value 39 of housekeeping block is currently reserved
    "",                       // value 40 of housekeeping block is currently reserved
    "",                       // value 41 of housekeeping block is currently reserved
    ""                        // value 42 of housekeeping block is currently reserved
};

static int numHousekeepingFields = sizeof(housekeepingFields)/sizeof(char*);

class ScubaSource::Config {
  public:
    Config() {
      _readMatrices = true;
      _validateChecksum = true;
      _rawDataBufferSize = RAW_DATA_BUFFER_SIZE;
      _rawDataCurtailAtBufferSize = true;
    }

    void read(KConfig *cfg, const QString& fileName = QString::null) {
      cfg->setGroup("SCUBA General");
      _readMatrices = cfg->readBoolEntry("Read Matrices", false);
      _validateChecksum = cfg->readBoolEntry("Validate Checksum", false);
      _rawDataBufferSize = cfg->readNumEntry("Raw Data Buffer Size", RAW_DATA_BUFFER_SIZE);
      _rawDataCurtailAtBufferSize = cfg->readBoolEntry("Raw Data Curtail To Buffer", true);

      if (!fileName.isEmpty()) {
        cfg->setGroup(fileName);
        _readMatrices = cfg->readBoolEntry("Read Matrices", _readMatrices);
        _validateChecksum = cfg->readBoolEntry("Validate Checksum", _validateChecksum);
        _rawDataBufferSize = cfg->readNumEntry("Raw Data Buffer Size", RAW_DATA_BUFFER_SIZE);
        _rawDataCurtailAtBufferSize = cfg->readBoolEntry("Raw Data Curtail To Buffer", true);
      }
    }

    bool _readMatrices;
    bool _validateChecksum;
    int _rawDataBufferSize;
    bool _rawDataCurtailAtBufferSize;

    void save(QTextStream& str, const QString& indent) {
      if (_readMatrices) {
        str << indent << "<matrices/>";
      }
      if (_validateChecksum) {
        str << indent << "<checksum/>";
      }
      str << indent << "<rawdata buffersize=\"" << _rawDataBufferSize << "\"";
      if (_rawDataCurtailAtBufferSize) {
        str << " curtail=\"1\"";
      }
      str << "/>" << endl;
    }

    void load(const QDomElement& e) {
      QDomNode n = e.firstChild();
      while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
          if (e.tagName() == "matrices") {
            _readMatrices = true;
          } else if (e.tagName() == "checksum") {
            _validateChecksum = true;
          } else if (e.tagName() == "rawdatacurtail") {
            _rawDataCurtailAtBufferSize = true;
          } else if (e.tagName() == "rawdata") {
            if (e.hasAttribute("buffersize")) {
              _rawDataBufferSize = e.attribute("buffersize").toInt();
              if (_rawDataBufferSize <= 0 ) {
                _rawDataBufferSize = RAW_DATA_BUFFER_SIZE;
              }
            } else {
              _rawDataBufferSize = RAW_DATA_BUFFER_SIZE;
            }

            if (e.hasAttribute("curtail")) {
              _rawDataCurtailAtBufferSize = true;
            } else {
              _rawDataCurtailAtBufferSize = false;
            }
          }
        }
        n = n.nextSibling();
      }
   }
};


ScubaSource::ScubaSource(KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e)
: KstDataSource(cfg, filename, type), _frameIndex(0L), _config(0L), _tmpBuf(0L), _tmpBufSize(0) {
  _valid = false;
  _haveHeader = false;
  _fieldListComplete = false;
  _rowStart = 0;
  _colStart = 0;
  _numCols = COLUMNS_PER_READOUTCARD;
  _numRows = 41;
  _rowLen = 0;
  _format = FormatText2;
  _numHousekeepingFieldsInUse = 0;
  _first = true;
  _numFramesLastReadMatrix = 0;
  _version = -1;

  for (int i=0; i<numHousekeepingFields; i++) {
    if (strlen(housekeepingFields[i]) > 0) {
      _numHousekeepingFieldsInUse++;
    }
  }

  if (type.isEmpty() || type == "SCUBA") {
    _config = new ScubaSource::Config;
    _config->read(cfg, filename);
    if (!e.isNull()) {
      _config->load(e);
    }

    _valid = true;
    update();
  }
}


ScubaSource::~ScubaSource() {
  if (_tmpBuf) {
    free(_tmpBuf);
    _tmpBuf = 0L;
    _tmpBufSize = 0;
  }

  if (_frameIndex) {
    free(_frameIndex);
    _frameIndex = 0L;
    _numFrameIndexAlloc = 0;
  }

  delete _config;
  _config = 0L;
}


bool ScubaSource::reset() {
  if (_tmpBuf) {
    free(_tmpBuf);
    _tmpBuf = 0L;
    _tmpBufSize = 0;
  }

  if (_frameIndex) {
    free(_frameIndex);
    _frameIndex = 0L;
    _numFrameIndexAlloc = 0;
  }

  _haveHeader = false;
  _fieldListComplete = false;
  _fieldList.clear();

  update();

  return true;
}


int ScubaSource::readFullLine(QFile &file, QString &str) {
  int read = file.readLine(str, 1000);

  if (read == 1000-1) {
    QString strExtra;
    while (str[read-1] != '\n') {
      int readExtra = file.readLine(strExtra, 1000);
      if (readExtra > 0) {
        read += readExtra;
        str += strExtra;
      } else {
        read = readExtra;
        break;
      }
    }
  }

  return read;
}


QString ScubaSource::runFile( const QString& filename ) {
  QString runfile;

  runfile = QString("%1.%2").arg(filename).arg("run");
  if (!QFile::exists(runfile)) {
    int index;

    runfile.truncate(0);
    index = filename.findRev(QChar('.'));
    if (index != -1) {
      runfile = QString("%1.%2").arg(filename.left(index)).arg("run");
      if (!QFile::exists(runfile)) {
        runfile.truncate(0);
      }
    }
  }

  return runfile;
}


void ScubaSource::setDataType(QFile& file) {
  int length = 200;
  int read;
  char text[length];

  read = file.readBlock((char*)text, length);
  if (read == length) {
    int i;

    _format = FormatText2;

    for (i=0; i<read; i++) {
      if (!isdigit(text[i]) && !isspace(text[i]) && text[i] != '\n') {
        _format = FormatBinary;

        break;
      }
    }
  }
}

bool ScubaSource::initFrameIndex() {
  bool rc = false;

  if (!_frameIndex) {
    _frameIndex = (QIODevice::Offset*)malloc(FRAMEINDEXBUFFERINCREMENT * sizeof(QIODevice::Offset));
    _numFrameIndexAlloc = FRAMEINDEXBUFFERINCREMENT;
  }

  _frameIndex[0] = 0;
  _byteLength = 0;
  _numFrames = 0;
  _readoutCards.clear();
  _datamodes.clear();
  _version = -1;

  QString runFilename = runFile(_filename);
  QFile file(_filename);

  //
  // check for the presence of a .run file
  //
  if (!runFilename.isEmpty()) {
    QFile frun(runFilename);
    KstString *metaString;
    QStringList readoutCardStrs;
    QStringList entries;
    QString metaName;
    QString scopy;
    QString s;
    bool done = false;
    bool ok;
    bool foundNumRows = false;
    bool foundRowLen = false;
    bool foundVersion = false;
    int readoutCard;
    int index;
    int line = 0;
    int read;

    _frameIndex[0] = 0;
    if (file.open(IO_ReadOnly)) {
      setDataType(file);
      file.close();
    }

    if (frun.open(IO_ReadOnly)) {
      rc = true;

      while (!done) {
        read = readFullLine(frun, s);
        if (read < 0) {
          done = true;
        } else {
          if (_first) {
            metaName.sprintf("LINE%03d", line);
            metaString = new KstString(KstObjectTag(metaName, tag()), this, s);
            _metaData.insert(metaName, metaString);
          }

          if (s.contains("data_mode") == 1) {
            index = s.find(QChar('>'));
            scopy = s;
            s.remove(0, index+1);
            s.stripWhiteSpace();
            _datamode = s.toInt(&ok, 10);
            if (!ok) {
              _datamode = -1;
            } else {
              if (scopy.contains(" rc") == 1) {
                index = scopy.find(QString(" rc"));
                scopy.remove(0, index+strlen(" rc"));
                index = scopy.find(QChar(' '));
                scopy.truncate(index);
                readoutCard = scopy.toInt(&ok, 10);
                if (ok) {
                  _datamodes[readoutCard] = (DataMode)_datamode;
                }
              }
            }
          } else if (!foundRowLen && s.contains("row_len") == 1) {
            index = s.find(QChar('>'));
            s.remove(0, index+1);
            s.stripWhiteSpace();
            _rowLen = s.toInt(&ok, 10);
            if (!ok) {
              _rowLen = -1;
            }
            foundRowLen = true;
          } else if (!foundNumRows && s.contains("num_rows") == 1) {
            index = s.find(QChar('>'));
            s.remove(0, index+1);
            s.stripWhiteSpace();
            _numRows = s.toInt(&ok, 10);
            if (!ok) {
              _numRows = -1;
            }
            foundNumRows = true;
          } else if (!foundVersion && s.contains("<DAS_VERSION>") == 1) {
            _version = readVersionNumber(s);
            foundVersion = true;
          } else if (s.contains("<RC>") == 1) {
            index = s.find(QChar('>'));
            s.remove(0, index+1);
            s.stripWhiteSpace();
            readoutCardStrs = QStringList::split(QChar(' '), s);
            for (QStringList::ConstIterator it = readoutCardStrs.begin(); it != readoutCardStrs.end(); ++it) {
              readoutCard = (*it).toInt(&ok, 16);
              if (ok) {
                _readoutCards.append(readoutCard);
              }
            }
          }

          ++line;
        }
      }

      _first = false;

      frun.close();
    }
  } else if (file.open(IO_ReadOnly)) {
    KstString *metaString;
    QStringList entries;
    QString metaName;
    QString s;
    bool done = false;
    bool ok;
    int line = 0;
    int read;
    int row;

    while (!done) {
      read = readFullLine(file, s);
      if (read < 0) {
        done = true;
      } else {
        if (_first) {
          metaName.sprintf("LINE%03d", line);
          metaString = new KstString(KstObjectTag(metaName, tag()), this, s);
          _metaData.insert(metaName, metaString);
        }

        if (s.compare(END_HEADER_1) == 0) {
          _frameIndex[0] = file.at();
          done = true;
          rc = true;
        } else if (s.contains("data_mode") == 1) {
          _datamode = s.right(8).toInt(&ok, 16);
          if (!ok) {
            _datamode = -1;
          }
        } else if (s.contains("row_len") == 1) {
          _rowLen = s.right(8).toInt(&ok, 16);
          if (!ok) {
            _rowLen = -1;
          }
        } else if (s.contains("num_rows") == 1) {
          _numRows = s.right(8).toInt(&ok, 16);
          if (!ok) {
            _numRows = -1;
          }
        } else if (s.contains("row_order") == 1) {
          entries = QStringList::split( QChar('\t'), s);
          if (!entries.empty()) {
            s = entries.last();
            entries = QStringList::split( QChar(' '), s);
            for (QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it) {
              row = (*it).toInt(&ok, 16);
              if (ok) {
                _rows.append(row);
              }
            }
          }
        }
        ++line;
      }
    }

    if (s.compare(END_HEADER_1) == 0) {
      setDataType(file);
    }

    _first = false;

    file.close();
  }

  if (_readoutCards.size() > 0) {
    _numCols = _readoutCards.size() * 8;
  }

  return rc;
}


KstObject::UpdateType ScubaSource::update(int u) {
  KstObject::UpdateType rc = lastUpdateResult();

  if (!KstObject::checkUpdateCounter(u)) {
    if (!_haveHeader) {
      _haveHeader = initFrameIndex();
      if (_haveHeader) {
        //
        // update the field list since we have one now
        //
        _fields = fieldListFor(_filename, _config);
        _fieldListComplete = _fields.count() > 1;

        //
        // update the matrix list since we have one now
        //
        _matrixList = matrixList();
      }
    }

    if (_haveHeader) {
      bool forceUpdate = false;
      QFile file(_filename);

      if (file.exists()) {
        if (uint(_byteLength) != file.size() || !_valid) {
          forceUpdate = true;
        }

        _byteLength = file.size();

        if (file.open(IO_ReadOnly)) {
          bool new_data = false;

          file.at(_frameIndex[_numFrames]);

          if (_format == FormatText) {
            //
            // currently not operational...
            //
          } else if (_format == FormatBinary) {
            long length = numHousekeepingFields + ( _numRows * _numCols );
            long values[length];
            long value;
            long checksum;
            long read = 0;
            int i;

            while (read != -1) {
              checksum = 0;

              read = file.readBlock((char*)values, length * sizeof( long ) );
              if (read == length * (long)sizeof( long ) ) {
                if (_config->_validateChecksum) {
                  for (i=0; i<length; i++) {
                    checksum ^= values[i];
                  }
                }
              } else {
                read = -1;
              }

              if (read != -1) {
                //
                // read the checksum value...
                //
                read = file.readBlock((char*)(&value), sizeof( long ) );
                if (read == sizeof( long ) ) {
                  if (!_config->_validateChecksum || value == checksum) {
                    if (_numFrames >= _numFrameIndexAlloc) {
                      _numFrameIndexAlloc += FRAMEINDEXBUFFERINCREMENT;
                      _frameIndex = (QIODevice::Offset*)realloc(_frameIndex, _numFrameIndexAlloc*sizeof(QIODevice::Offset));
                    }

                    new_data = true;
                    ++_numFrames;
                    _frameIndex[_numFrames] = file.at();
                  }
                } else {
                  read = -1;
                }
              }
            }
          } else if (_format == FormatText2) {
            QStringList entries;
            QString s;
            long value;
            long checksum;
            bool ok;
            int read = 0;
            int i;

            while (read != -1) {
              checksum = 0;
              for (i=0; i<_numRows+1; i++) {
                read = readFullLine(file, s);

                if (read == -1) {
                  break;
                }

                if (_config->_validateChecksum) {
                  entries = QStringList::split(QChar(' '), s);
                  for (QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it) {
                    value = (*it).toInt(&ok, 10);
                    if (ok) {
                      checksum ^= value;
                    }
                  }
                }
              }

              if (read != -1) {
                read = readFullLine(file, s);
                if (read != -1) {
                  value = s.toInt(&ok, 10);
                  if (!_config->_validateChecksum || value == checksum) {
                    if (_numFrames >= _numFrameIndexAlloc) {
                      _numFrameIndexAlloc += FRAMEINDEXBUFFERINCREMENT;
                      _frameIndex = (QIODevice::Offset*)realloc(_frameIndex, _numFrameIndexAlloc*sizeof(QIODevice::Offset));
                    }
                    new_data = true;
                    ++_numFrames;
                    _frameIndex[_numFrames] = file.at();
                  }
                }
              }
            }
          }

          file.close();

          updateNumFramesScalar();

          _valid = true;

          rc = setLastUpdateResult(forceUpdate ? KstObject::UPDATE : (new_data ? KstObject::UPDATE : KstObject::NO_CHANGE));
        } else {
          _valid = false;

          rc = setLastUpdateResult(KstObject::NO_CHANGE);
        }
      } else {
        _valid = false;

        rc = setLastUpdateResult(KstObject::NO_CHANGE);
      }
    } else {
      rc = setLastUpdateResult(KstObject::NO_CHANGE);
    }
  }

  return rc;
}


bool ScubaSource::fieldListIsComplete() const {
  return _fieldListComplete;
}


bool ScubaSource::matrixDimensions(const QString& matrix, int* xDim, int* yDim) {
  bool rc = false;

  if (isValidMatrix(matrix)) {
    rc = true;

    *xDim = _numCols;
    *yDim = _numRows;
  }

  return rc;
}


int ScubaSource::readMatrix(KstMatrixData* data, const QString& matrix, int xStart, int yStart, int xNumSteps, int yNumSteps) {
  int totalSamples = 0;

  if (isValidMatrix(matrix)) {
    if (xNumSteps > 0 && yNumSteps > 0) {
      if (_datamode != DataRaw) {
        QFile file(_filename);

        if (file.open(IO_ReadOnly)) {
          double *values = data->z;
          long lvalue = 0;
          bool ok;
          bool error = false;
          bool max = false;
          bool min = false;
          bool avg = false;
          int read = 0;
          int frameStart = _numFrames;
          int frameEnd = _numFrames;
          int frame;
          int i;
          int j;

          if (matrix.contains("_") > 0) {
            QString str = matrix;
            int index = str.find(QChar('_'));

            str.remove(0, index+1);
            frameStart = str.toInt(&ok, 10);
            if (ok) {
              frameStart++;
              frameEnd = frameStart;
            } else {
              frameStart = _numFrames;
              frameEnd = _numFrames;
            }
          }

          if (matrix.contains("Error") > 0) {
            error = true;
          }

          if (matrix.contains("Max") > 0) {
           frameStart = _numFramesLastReadMatrix;
           max = true;
          } else if (matrix.contains("Min") > 0) {
           frameStart = _numFramesLastReadMatrix;
           min = true;
          } else if (matrix.contains("Avg") > 0) {
            frameStart = _numFramesLastReadMatrix;
            avg = true;
          }

          for (frame=frameStart-1; frame<frameEnd; frame++) {
            file.at(_frameIndex[frame]);

            if (_format == FormatText) {
              //
              // currently not operational...
              //
            } else if (_format == FormatBinary) {
              //
              // always need to skip the housekeeping fields...
              //
              int length = numHousekeepingFields + ( _numRows * _numCols );
              long rawValues[length];
              long read;

              read = file.readBlock((char*)rawValues, length * sizeof( long ));
              if( read == length * (long)sizeof( long )) {
                for (i=0; i<xNumSteps; ++i) {
                  if (i < _numCols) {
                    for (j=0; j<yNumSteps; ++j) {
                      if (j < _numRows) {
                        int rowIndex = j;
                        int colIndex = i;

                        if (_version <= 111) {
                          rowIndex += _numRows * ( colIndex / COLUMNS_PER_READOUTCARD );
                          colIndex %= COLUMNS_PER_READOUTCARD;
                        }

                        lvalue = rawValues[numHousekeepingFields + ( rowIndex * _numCols ) + colIndex];
                        switch (_datamode) {
                          case DataError:
                          case DataPreScaleFeedback:
                          case DataFiltered:
                          case DataRaw:
                            break;
                          case Data18_14:
                          case DataFiltered18_14:
                            if (!error) {
                              lvalue /= 0x4000;
                            } else {
                              lvalue &= 0x3FFF;
                              if (lvalue & 0x2000) {
                                lvalue -= 0x4000;
                              }
                            }
                            break;
                          case Data24_8:
                            if (!error) {
                              lvalue /= 0x100;
                            } else {
                              lvalue &= 0xFF;
                              if (lvalue & 0x80) {
                                lvalue -= 0x100;
                              }
                            }
                            break;
                        }

                        if (max) {
                          if (frame > frameStart && double(lvalue) > *values) {
                            *values = double(lvalue);
                          }
                        } else if (min) {
                          if (frame > frameStart && double(lvalue) < *values) {
                            *values = double(lvalue);
                          }
                        } else if (avg) {
                          *values += double(lvalue);
                        } else {
                          *values = double(lvalue);
                        }
                      }

                      ++values;
                    }
                  }
                }

              }
            } else if (_format == FormatText2) {
              QStringList lines;
              QStringList entries;
              QString str;
              int length = xNumSteps * yNumSteps;
              long rawValues[length];

              //
              // always need to skip the first line as it is the header...
              //
              for (i=0; i<yStart+1; ++i) {
                read = readFullLine(file, str);
              }

              for (i=0; i<yNumSteps; ++i) {
                read = readFullLine(file, str);
                if (read != -1) {
                  entries = QStringList::split(QChar(' '), str);
                  for (j=0; j<xNumSteps; j++) {
                    if (j < int(entries.size())) {
                      lvalue = entries[j].toInt(&ok, 10);
                      if (ok) {
                        rawValues[(i * xNumSteps) + j] = lvalue;
                      } else {
                        rawValues[(i * xNumSteps) + j] = 0;
                      }
                    }
                  }
                }
              }

              for (i=0; i<xNumSteps; ++i) {
                for (j=0; j<yNumSteps; ++j) {
                  if ((j * xNumSteps) + i < length) {
                    lvalue = rawValues[(j * xNumSteps) + i];

                    switch (_datamode) {
                      case DataError:
                      case DataPreScaleFeedback:
                      case DataFiltered:
                      case DataRaw:
                        break;
                      case Data18_14:
                      case DataFiltered18_14:
                        if (!error) {
                          lvalue /= 0x4000;
                        } else {
                          lvalue &= 0x3FFF;
                          if (lvalue & 0x2000) {
                            lvalue -= 0x4000;
                          }
                        }
                        break;
                      case Data24_8:
                        if (!error) {
                          lvalue /= 0x100;
                        } else {
                          lvalue &= 0xFF;
                          if (lvalue & 0x80) {
                            lvalue -= 0x100;
                          }
                        }
                        break;
                    }

                    if (max) {
                      if (frame > frameStart && double(lvalue) > *values) {
                        *values = double(lvalue);
                      }
                    } else if (avg) {
                      *values += double(lvalue);
                    } else {
                      *values = double(lvalue);
                    }

                    ++values;
                  }
                }
              }
            }
          }

          if (avg && frameEnd > frameStart) {
            values = data->z;

            for (i=0; i<yNumSteps; ++i) {
              for (j=0; j<xNumSteps; j++) {
                *values /= double(frameEnd - frameStart);
                ++values;
              }
            }
          }

          data->xMin = xStart;
          data->yMin = yStart;
          data->xStepSize = 1;
          data->yStepSize = 1;

          totalSamples = xNumSteps * yNumSteps;

          _numFramesLastReadMatrix = _numFrames;

          file.close( );
        }
      }
    }
  }

  return totalSamples;
}


int ScubaSource::readField(double *v, const QString& field, int s, int n) {
  DataMode datamode = DataInvalid;
  bool isError = false;
  int fieldIndex = -1;
  int frameIndex = -1;
  int rowIndex = -1;
  int colIndex = -1;
  int rc = 0;
  int i = 0;

  if (n < 0) {
    n = 1;
  }

  //
  // check if the field is the INDEX...
  //
  if (field == "INDEX") {
    for (i = 0; i < n; i++) {
      v[i] = double(s + i);

      rc = n;
    }
  } else {
    //
    // check if the field is a housekeeping field...
    //
    for (i=0; i < _numHousekeepingFieldsInUse; i++) {
      if (field.compare(housekeepingFields[i]) == 0) {
        fieldIndex = i;

        break;
      }
    }

    //
    // check if the field is a Pixel_i_j or Error_i_j entry...
    //
    if (fieldIndex == -1) {
      QMap<int, DataMode>::Iterator datamodeIt;
      QString str = field;
      QString strRowCol;
      bool ok;
      int readoutCard;
      int index;

      if (str.find("Error") != -1) {
        isError = true;
      }

      index = str.find(QChar('_'));
      if (index != -1) {
        str.remove(0, index+1);
        index = str.find(QChar('_'));
        if (index != -1) {
          //
          // find the column index, the second value...
          //
          strRowCol = str;
          strRowCol.remove(0, index+1);
          colIndex = strRowCol.toInt(&ok, 10);

          if (ok) {
            //
            // find the row index, the first value...
            //
            strRowCol = str;
            strRowCol.remove(index, str.length());
            rowIndex = strRowCol.toInt(&ok, 10);
          }
        }
      }

      //
      // determine the data mode for the readout card asoicated with the field...
      //
      if (rowIndex != -1 && colIndex != -1) {
        readoutCard = (colIndex / COLUMNS_PER_READOUTCARD)+1;
        datamodeIt = _datamodes.find(readoutCard);
        if (datamodeIt != _datamodes.end()) {
          datamode = *datamodeIt;
        }

        //
        // we may need to change the colIndex if we are missing readout cards...
        //
        for (i=readoutCard-1; i>0 ; i--) {
          if (_readoutCards.find(i) == _readoutCards.end()) {
            colIndex -= COLUMNS_PER_READOUTCARD;
          }
        }

        //
        // determine the field index...
        //
        if (_version > 111 && _format == FormatBinary) {
          fieldIndex  = rowIndex * _readoutCards.size() * COLUMNS_PER_READOUTCARD;
          fieldIndex += colIndex;
        } else {
          fieldIndex  = _numRows * ( colIndex / COLUMNS_PER_READOUTCARD );
          fieldIndex += rowIndex * COLUMNS_PER_READOUTCARD;
          fieldIndex += colIndex % COLUMNS_PER_READOUTCARD;
        }
        fieldIndex += _numHousekeepingFieldsInUse;
      }
    }

    if (datamode == DataInvalid) {
      datamode = (DataMode)_datamode;
    }

    if (fieldIndex != -1) {
      int iSamplesPerFrame = samplesPerFrame(field);

      QIODevice::Offset bufread = _frameIndex[(s + n)/iSamplesPerFrame] - _frameIndex[s/iSamplesPerFrame];

      if (bufread > 0) {
        QFile file(_filename);
        long lvalue = 0;
        int valueIndex = 0;

        if (file.open(IO_ReadOnly)) {
          if (_format == FormatText) {
            //
            // currently not operational...
            //
          } else if (_format == FormatBinary) {
            long read = 0;
            long length;

            for (i = 0; i < n; ++i) {
              v[i] = KST::NOPOINT;

              if (fieldIndex < _numHousekeepingFieldsInUse) {
                long values[_numHousekeepingFieldsInUse];

                file.at(_frameIndex[(s + i)/iSamplesPerFrame]);

                length = fieldIndex + 1;
                read = file.readBlock((char*)values, length * sizeof( long ) );
                if( read == length * (long)sizeof( long ) ) {
                  v[i] = double(values[fieldIndex]);
                }
              } else {
                switch (_datamode) {
                  case DataError:
                  case DataPreScaleFeedback:
                  case DataFiltered:
                    file.at(_frameIndex[(s + i)/iSamplesPerFrame]);
                    valueIndex = fieldIndex - _numHousekeepingFieldsInUse;
                    break;
                  case DataRaw:
                    {
                      int burstOffset;
                      int lineOffset;
                      int rowOffset;

                      //
                      // first we must determine the following:
                      //  burst offset:  which acoounts for the frame index contributon from the burst index
                      //  row offset:    which accounts for the frame index contribution from the row
                      //  line offset:   which accounts for the frame index contribution from the desired value
                      //
                      burstOffset = _rowLen * _numRows * ( (s + i) / _rowLen );
                      rowOffset = _rowLen * ( ( fieldIndex - _numHousekeepingFieldsInUse ) / _numCols );
                      lineOffset = ( s + i ) % _rowLen;
                      frameIndex = ( burstOffset + rowOffset + lineOffset ) / _numRows;
                      file.at(_frameIndex[frameIndex]);

                      valueIndex = _numCols * ( ( burstOffset + rowOffset + lineOffset ) % _numRows );
                      valueIndex += ( fieldIndex - _numHousekeepingFieldsInUse ) % _numCols;
                    }
                    break;
                  case Data18_14:
                  case Data24_8:
                  case DataFiltered18_14:
                    file.at(_frameIndex[(s + i)/iSamplesPerFrame]);
                    valueIndex = fieldIndex - _numHousekeepingFieldsInUse;
                    break;
                }

                length = numHousekeepingFields + valueIndex + 1;
                long values[length];
                read = file.readBlock((char*)values, length * sizeof( long ) );
                if (read == length * (long)sizeof( long ) ) {
                  lvalue = values[numHousekeepingFields + valueIndex];

                  switch (_datamode) {
                    case DataError:
                    case DataPreScaleFeedback:
                    case DataFiltered:
                      break;
                    case DataRaw:
                      lvalue &= 0xFF;
                      break;
                    case Data18_14:
                    case DataFiltered18_14:
                      if (!isError) {
                        lvalue /= 0x4000;
                      } else {
                        lvalue &= 0x3FFF;
                        if (lvalue & 0x2000) {
                          lvalue -= 0x4000;
                        }
                      }
                      break;
                    case Data24_8:
                      if (!isError) {
                        lvalue /= 0x100;
                      } else {
                        lvalue &= 0xFF;
                        if (lvalue & 0x80) {
                          lvalue -= 0x100;
                        }
                      }
                      break;
                  }
                  v[i] = double(lvalue);
                }
              }
            }

            rc = n;
          } else if (_format == FormatText2) {
            QStringList values;
            QString str;
            bool ok;
            int lineIndex;
            int lines;
            int read = 0;

            for (i = 0; i < n; ++i) {
              v[i] = KST::NOPOINT;

              if (fieldIndex < _numHousekeepingFieldsInUse) {
                file.at(_frameIndex[(s + i)/iSamplesPerFrame]);

                if( readFullLine(file, str) != -1) {
                  values = QStringList::split(QChar(' '), str);
                  if (fieldIndex < int(values.size())) {
                    v[i] = double(values[fieldIndex].toInt(&ok, 10));
                    if (!ok) {
                      v[i] = KST::NOPOINT;
                    }
                  }
                }
              } else {
                switch (_datamode) {
                  case DataError:
                  case DataPreScaleFeedback:
                  case DataFiltered:
                    file.at(_frameIndex[(s + i)/iSamplesPerFrame]);
                    valueIndex = fieldIndex - _numHousekeepingFieldsInUse;
                    break;
                  case DataRaw:
                    {
                      int burstOffset;
                      int lineOffset;
                      int rowOffset;

                      //
                      // first we must determine the following:
                      //  burst offset:  which acoounts for the frame index contributon from the burst index
                      //  row offset:    which accounts for the frame index contribution from the row
                      //  line offset:   which accounts for the frame index contribution from the desired value
                      //
                      burstOffset = _rowLen * _numRows * ( (s + i) / _rowLen );
                      rowOffset = _rowLen * ( ( fieldIndex - _numHousekeepingFieldsInUse ) / _numCols );
                      lineOffset = ( s + i ) % _rowLen;
                      frameIndex = ( burstOffset + rowOffset + lineOffset ) / _numRows;
                      file.at(_frameIndex[frameIndex]);

                      valueIndex = _numCols * ( ( burstOffset + rowOffset + lineOffset ) % _numRows );
                      valueIndex += ( fieldIndex - _numHousekeepingFieldsInUse ) % _numCols;
                    }
                    break;
                  case Data18_14:
                  case Data24_8:
                  case DataFiltered18_14:
                    file.at(_frameIndex[(s + i)/iSamplesPerFrame]);
                    valueIndex = fieldIndex - _numHousekeepingFieldsInUse;
                    break;
                }

                lines = valueIndex / COLUMNS_PER_TEXTFORMAT;
                lineIndex = valueIndex - ( lines * COLUMNS_PER_TEXTFORMAT );

                //
                // always need to skip the first line as it is the header...
                //
                for (int j=0; j<lines+1; j++) {
                  readFullLine(file, str);
                }

                if (read != -1) {
                  if (readFullLine(file, str) != -1) {
                    values = QStringList::split(QChar(' '), str);
                    if (lineIndex < int(values.size())) {
                      lvalue = values[lineIndex].toInt(&ok, 10);
                      switch (_datamode) {
                        case DataError:
                        case DataPreScaleFeedback:
                        case DataFiltered:
                          break;
                        case DataRaw:
                          lvalue &= 0xFF;
                          break;
                        case Data18_14:
                        case DataFiltered18_14:
                          if (!isError) {
                            lvalue /= 0x4000;
                          } else {
                            lvalue &= 0x3FFF;
                            if (lvalue & 0x2000) {
                              lvalue -= 0x4000;
                            }
                          }
                          break;
                        case Data24_8:
                          if (!isError) {
                            lvalue /= 0x100;
                          } else {
                            lvalue &= 0xFF;
                            if (lvalue & 0x80) {
                              lvalue -= 0x100;
                            }
                          }
                          break;
                      }
                      v[i] = double(lvalue);
                    }
                  }
                }
              }
            }

            rc = n;
          }

          file.close();
        } else {
          _valid = false;

          rc = 0;
        }
      }
    }
  }

  return rc;
}


bool ScubaSource::isValidField(const QString& field) const {
  return fieldList().contains(field);
}


bool ScubaSource::isValidMatrix(const QString& matrix) const {
  return matrixList().contains(matrix);
}


int ScubaSource::samplesPerFrame(const QString &field) {
  //
  // the number of samples per frame is the same for all fields...
  //
  Q_UNUSED(field)

  int rc = 1;

  return rc;
}


int ScubaSource::frameCount(const QString& field) const {
  int numFrames = -1;

  if (_datamode == DataRaw) {
    int i;

    //
    // if we have a housekeeping field then all the frames are valid...
    //
    for (i=0; i<numHousekeepingFields; i++) {
      if (field.compare(housekeepingFields[i]) == 0) {
        numFrames = _numFrames;
        break;
      }
    }

    if (numFrames == -1) {
      numFrames = _numFrames;

      //
      // allow for the limited buffer size...
      //
      if (_config->_rawDataCurtailAtBufferSize) {
        if (_numFrames * _numRows * _numCols > _config->_rawDataBufferSize) {
          int numBursts;

          numBursts = _config->_rawDataBufferSize / ( _numRows * _numCols * _rowLen );
          numFrames = numBursts * _rowLen;
        }
      }
    }
  } else {
    numFrames = _numFrames;
  }

  return numFrames;
}


QString ScubaSource::fileType() const {
  return "SCUBA";
}


bool ScubaSource::isEmpty() const {
  return _numFrames < 1;
}

QStringList ScubaSource::fieldListFor(const QString& filename, ScubaSource::Config *cfg) {
  Q_UNUSED(cfg)

  QStringList readoutCardStrs;
  QValueList<int> readoutCards;
  QStringList rc;
  QFile file(filename);
  QString runFilename;
  QMap<int, DataMode> datamodes;
  bool populate = false;
  int datamode = -1;
  int num_rows = -1;
  int num_cols = -1;
  int version = -1;

  //
  // check for the presence of a .run file
  //
  runFilename = runFile(filename);
  if (!runFilename.isEmpty()) {
    QFile frun(runFilename);

    if (frun.open(IO_ReadOnly)) {
      QString scopy;
      QString s;
      bool foundNumRows = false;
      bool foundVersion = false;
      bool done = false;
      bool ok;
      int readoutCard;
      int index;
      int read;

      populate = true;

      while (!done) {
        read = frun.readLine(s, 1000);
        if (read < 0) {
          done = true;
        } else if (s.compare(END_HEADER_1) == 0) {
          done = true;
        } else if (s.contains("data_mode") == 1) {
          scopy = s;
          index = s.find(QChar('>'));
          s.remove(0, index+1);
          s.stripWhiteSpace();
          datamode = s.toInt(&ok, 10);
          if (!ok) {
            datamode = -1;
          } else if (scopy.contains(" rc") == 1) {
            index = scopy.find(QString(" rc"));
            scopy.remove(0, index+strlen(" rc"));
            index = scopy.find(QChar(' '));
            scopy.truncate(index);
            readoutCard = scopy.toInt(&ok, 10);
            if (ok) {
              datamodes.insert(readoutCard, (DataMode)datamode);
            }
          }
        } else if (!foundNumRows && s.contains("num_rows") == 1) {
          index = s.find(QChar('>'));
          s.remove(0, index+1);
          s.stripWhiteSpace();
          num_rows = s.toInt(&ok, 10);
          if (!ok) {
            num_rows = -1;
          }
          foundNumRows = true;
        } else if (!foundVersion && s.contains("DAS_VERSION") == 1) {
          version = readVersionNumber(s);
          foundVersion = true;
        } else if (s.contains("<RC>") == 1) {
          readoutCards.clear();
          index = s.find(QChar('>'));
          s.remove(0, index+1);
          s.stripWhiteSpace();
          readoutCardStrs = QStringList::split(QChar(' '), s);
          for (QStringList::ConstIterator it = readoutCardStrs.begin(); it != readoutCardStrs.end(); ++it) {
            readoutCard = (*it).toInt(&ok, 10);
            if (ok) {
              readoutCards.append(readoutCard);
            }
          }
        }
      }

      frun.close();
    }
  } else if (file.open(IO_ReadOnly)) {
    QStringList entries;
    QValueList<int> rows;
    QString s;
    bool done = false;
    bool ok;
    int read;

    populate = true;

    while (!done) {
      ok = true;
      read = file.readLine(s, 1000);
      if (read < 0) {
        done = true;
      } else if (s.compare(END_HEADER_1) == 0) {
        done = true;
      } else if (s.contains("data_mode") == 1) {
        datamode = s.right(8).toInt(&ok, 16);
        if (!ok) {
          datamode = -1;
        }
      } else if (s.contains("num_rows") == 1) {
        num_rows = s.right(8).toInt(&ok, 16);
        if (!ok) {
          num_rows = -1;
        }
      }
    }

    file.close();
  }

  if (populate) {
    int i;
    int j;
    int k;

    //
    // if we didn't find any <RC> entries then assume that only readout card 1 is active...
    //
    if (readoutCards.size() == 0) {
      readoutCards.append(1);
    }

    num_cols = readoutCards.size() * COLUMNS_PER_READOUTCARD;

    rc += "INDEX";

    for (i=0; i<numHousekeepingFields; i++) {
      if (strlen(housekeepingFields[i]) > 0) {
        rc += housekeepingFields[i];
      }
    }

    if (datamodes.size() > 0 && readoutCards.size() > 0) {
      for (i=0; i<num_rows; i++) {
        for (j=0; j<(int)readoutCards.size(); j++) {
          int readoutCard = readoutCards[j];
          QMap<int, DataMode>::Iterator datamodesIt = datamodes.find(readoutCard);

          if (datamodesIt != datamodes.end()) {
            datamode = *datamodesIt;
            if (datamode >= DataError && datamode < DataInvalid) {
              for (k=0; k<COLUMNS_PER_READOUTCARD; k++) {
                int row = i;
                int col = ((readoutCard-1)*COLUMNS_PER_READOUTCARD)+k;

                switch (datamode) {
                  case DataError:
                    rc += QString("Error_%1_%2").arg(row).arg(col);
                    break;
                  case DataPreScaleFeedback: 
                    rc += QString("Pixel_%1_%2").arg(row).arg(col);
                    break;
                  case DataFiltered:
                    rc += QString("Pixel_%1_%2").arg(row).arg(col);
                    break;
                  case DataRaw:
                    rc += QString("Pixel_%1_%2").arg(row).arg(col);
                    break;
                  case Data18_14:
                    rc += QString("Pixel_%1_%2").arg(row).arg(col);
                    rc += QString("Error_%1_%2").arg(row).arg(col);
                    break;
                  case Data24_8: 
                    rc += QString("Pixel_%1_%2").arg(row).arg(col);
                    rc += QString("Error_%1_%2").arg(row).arg(col);
                    break;
                  case DataFiltered18_14:
                    rc += QString("Pixel_%1_%2").arg(row).arg(col);
                    rc += QString("Error_%1_%2").arg(row).arg(col);
                    break;
                  default:
                    break;
                }
              }
            }
          }
        }
      }
    } else if (datamode >= DataError) {
      for (i=0; i<num_rows; i++) {
        for (j=0; j<num_cols; j++) {
          switch (datamode) {
            case DataError:
              rc += QString("Error_%1_%2").arg(i).arg(j);
              break;
            case DataPreScaleFeedback: 
              rc += QString("Pixel_%1_%2").arg(i).arg(j);
              break;
            case DataFiltered:
              rc += QString("Pixel_%1_%2").arg(i).arg(j);
              break;
            case DataRaw:
              rc += QString("Pixel_%1_%2").arg(i).arg(j);
              break;
            case Data18_14:
              rc += QString("Pixel_%1_%2").arg(i).arg(j);
              rc += QString("Error_%1_%2").arg(i).arg(j);
              break;
            case Data24_8: 
              rc += QString("Pixel_%1_%2").arg(i).arg(j);
              rc += QString("Error_%1_%2").arg(i).arg(j);
              break;
            case DataFiltered18_14: 
              rc += QString("Pixel_%1_%2").arg(i).arg(j);
              rc += QString("Error_%1_%2").arg(i).arg(j);
              break;
            default:
              break;
          }
        }
      }
    }
  }

  return rc;
}


QStringList ScubaSource::fieldList() const {
  if (_fields.isEmpty()) {
    _fields = fieldListFor(_filename, _config);
    _fieldListComplete = _fields.count() > 1;
  }

  return _fields;
}


QStringList ScubaSource::matrixList() const {
  QStringList readoutCardStrs;
  QValueList<int> readoutCards;
  QMap<int, DataMode> datamodes;
  bool populate = false;
  int datamode = -1;
  int num_rows = -1;
  int row_len = -1;
  int version = -1;

  _matrixList.clear();

  if (_config->_readMatrices) {
    if (_datamode != DataRaw) {
      QFile file(_filename);
      QString runFilename;

      runFilename = runFile(_filename);
      if (!runFilename.isEmpty()) {
        QFile frun(runFilename);

        if (frun.open(IO_ReadOnly)) {
          QString scopy;
          QString s;
          bool foundNumRows = false;
          bool foundRowLen = false;
          bool foundVersion = false;
          bool done = false;
          bool ok;
          int readoutCard;
          int index;
          int read;

          populate = true;

          while (!done) {
            read = frun.readLine(s, 1000);
            if (read < 0) {
              done = true;
            } else if (s.compare(END_HEADER_1) == 0) {
              done = true;
            } else if (s.contains("data_mode") == 1) {
              index = s.find(QChar('>'));
              scopy = s;
              s.remove(0, index+1);
              s.stripWhiteSpace();
              datamode = s.toInt(&ok, 10);
              if (!ok) {
                datamode = -1;
              } else if (scopy.contains(" rc") == 1) {
                index = scopy.find(QString(" rc"));
                scopy.remove(0, index+strlen(" rc"));
                index = scopy.find(QChar(' '));
                scopy.truncate(index);
                readoutCard = scopy.toInt(&ok, 10);
                if (ok) {
                  datamodes.insert(readoutCard, (DataMode)datamode);
                }
              }
            } else if (!foundRowLen && s.contains("row_len") == 1) {
              index = s.find(QChar('>'));
              s.remove(0, index+1);
              s.stripWhiteSpace();
              row_len = s.toInt(&ok, 10);
              if (!ok) {
                row_len = -1;
              }
              foundRowLen = true;
            } else if (!foundNumRows && s.contains("num_rows") == 1) {
              index = s.find(QChar('>'));
              s.remove(0, index+1);
              s.stripWhiteSpace();
              num_rows = s.toInt(&ok, 10);
              if (!ok) {
                num_rows = -1;
              }
              foundNumRows = true;
            } else if (!foundVersion && s.contains("<DAS_VERSION>") == 1) {
              version = readVersionNumber(s);
              foundVersion = true;
            } else if (s.contains("<RC>") == 1) {
              index = s.find(QChar('>'));
              s.remove(0, index+1);
              s.stripWhiteSpace();
              readoutCardStrs = QStringList::split(QChar(' '), s);
              for (QStringList::ConstIterator it = readoutCardStrs.begin(); it != readoutCardStrs.end(); ++it) {
                readoutCard = (*it).toInt(&ok, 16);
                if (ok) {
                  readoutCards.append(readoutCard);
                }
              }
            }
          }

          frun.close();
        }
      } else if (file.open(IO_ReadOnly)) {
        QStringList entries;
        QStringList rc;
        QValueList<int> rows;
        QString s;
        bool done = false;
        bool ok;
        int read;
        int row;

        populate = true;

        while (!done) {
          read = file.readLine(s, 1000);
          if (read < 0) {
            done = true;
          } else if (s.compare(END_HEADER_1) == 0) {
            done = true;
          } else if (s.contains("data_mode") == 1) {
            datamode = s.right(8).toInt(&ok, 16);
            if (!ok) {
              datamode = -1;
            }
          } else if (s.contains("row_len") == 1) {
            row_len = s.right(8).toInt(&ok, 16);
            if (!ok) {
              row_len = -1;
            }
          } else if (s.contains("num_rows") == 1) {
            num_rows = s.right(8).toInt(&ok, 16);
            if (!ok) {
              num_rows = -1;
            }
          } else if (s.contains("row_order") == 1) {
            entries = QStringList::split( QChar('\t'), s);
            if (!entries.empty()) {
              s = entries.last();
              entries = QStringList::split( QChar(' '), s);
              for (QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it) {
                row = (*it).toInt(&ok, 16);
                if (ok) {
                  rows.append(row);
                }
              }
            }
          }
        }

        file.close();
      }
    }
  }

  if (populate) {
    bool sameDataMode = true;
    int i;

    //
    // if we didn't find any <RC> entries then assume that only readout card 1 is active...
    //
    if (readoutCards.size() == 0) {
      readoutCards.append(1);
    }

    //
    // check that all the active cards are in the same mode...
    //
    for (i=0; i<(int)readoutCards.size(); i++) {
      int readoutCard = readoutCards[i];
      QMap<int, DataMode>::Iterator datamodesIt = datamodes.find(readoutCard);

      if (datamodesIt != datamodes.end()) {
        if (i == 0) {
          datamode = *datamodesIt;
        } else if (datamode != *datamodesIt) {
          sameDataMode = false;
        }
      }
    }

    if (sameDataMode) {
      switch (datamode) {
        case DataError:
          _matrixList += QString("FrameErrorLast");
          _matrixList += QString("FrameErrorRecentAvg");
          _matrixList += QString("FrameErrorRecentMax");
          break;
        case DataPreScaleFeedback:
          _matrixList += QString("FramePixelLast");
          _matrixList += QString("FramePixelRecentAvg");
          _matrixList += QString("FramePixelRecentMax");
          _matrixList += QString("FramePixelRecentMin");

          _matrixList += QString("FrameErrorLast");
          _matrixList += QString("FrameErrorRecentAvg");
          _matrixList += QString("FrameErrorRecentMax");
          break;
        case DataFiltered:
          _matrixList += QString("FramePixelLast");
          _matrixList += QString("FramePixelRecentAvg");
          _matrixList += QString("FramePixelRecentMax");
          _matrixList += QString("FramePixelRecentMin");
          break;
        case Data18_14:
          _matrixList += QString("FramePixelLast");
          _matrixList += QString("FramePixelRecentAvg");
          _matrixList += QString("FramePixelRecentMax");
          _matrixList += QString("FramePixelRecentMin");

          _matrixList += QString("FrameErrorLast");
          _matrixList += QString("FrameErrorRecentAvg");
          _matrixList += QString("FrameErrorRecentMax");
          break;
        case Data24_8:
          _matrixList += QString("FramePixelLast");
          _matrixList += QString("FramePixelRecentAvg");
          _matrixList += QString("FramePixelRecentMax");
          _matrixList += QString("FramePixelRecentMin");

          _matrixList += QString("FrameErrorLast");
          _matrixList += QString("FrameErrorRecentAvg");
          _matrixList += QString("FrameErrorRecentMax");
          break;
        case DataFiltered18_14:
          _matrixList += QString("FramePixelLast");
          _matrixList += QString("FramePixelRecentAvg");
          _matrixList += QString("FramePixelRecentMax");
          _matrixList += QString("FramePixelRecentMin");

          _matrixList += QString("FrameErrorLast");
          _matrixList += QString("FrameErrorRecentAvg");
          _matrixList += QString("FrameErrorRecentMax");
          break;
        default:
          break;
      }

      for (i=0; i<_numFrames; i++) {
        switch (datamode) {
          case DataError:
            _matrixList += QString("FrameError_%1").arg(i);
            break;
          case DataPreScaleFeedback: 
            _matrixList += QString("FramePixel_%1").arg(i);
            break;
          case DataFiltered:
            _matrixList += QString("FramePixel_%1").arg(i);
            break;
          case Data18_14:
            _matrixList += QString("FramePixel_%1").arg(i);
            _matrixList += QString("FrameError_%1").arg(i);
            break;
          case Data24_8:
            _matrixList += QString("FramePixel_%1").arg(i);
            _matrixList += QString("FrameError_%1").arg(i);
            break;
          case DataFiltered18_14:
            _matrixList += QString("FramePixel_%1").arg(i);
            _matrixList += QString("FrameError_%1").arg(i);
            break;
          default:
            break;
        }
      }
    }
  }

  return _matrixList;
}


void ScubaSource::save(QTextStream &ts, const QString& indent) {
  KstDataSource::save(ts, indent);
  _config->save(ts, indent);
}


bool ScubaSource::supportsTimeConversions() const {
  return false;
}


int ScubaSource::sampleForTime(double ms, bool *ok) {
  return 0;
}


int ScubaSource::sampleForTime(const KST::ExtDateTime& time, bool *ok) {
  return 0;
}

int ScubaSource::readVersionNumber(QString& s) {
  int index = s.find(QChar('>'));
  int version = -1;
  bool ok;

  s.remove(0, index+1);
  s = s.stripWhiteSpace();
  s.remove(4, s.length());
  version = s.toInt(&ok, 10);
  if (!ok) {
    version = -1;
  }

  return version;
}

class ConfigWidgetScuba : public KstDataSourceConfigWidget {
  public:
    ConfigWidgetScuba() : KstDataSourceConfigWidget() {
      QGridLayout *layout = new QGridLayout(this, 1, 1);
      _ac = new ScubaConfig(this);
      layout->addWidget(_ac, 0, 0);
      layout->activate();
    }

    virtual ~ConfigWidgetScuba() {}

    virtual void setConfig(KConfig *cfg) {
      KstDataSourceConfigWidget::setConfig(cfg);
    }

    virtual void load() {
      QString str;
      bool hasInstance = _instance != 0L;

      _cfg->setGroup("SCUBA General");
      _ac->_readMatrices->setChecked(_cfg->readBoolEntry("Read Matrices", false));
      _ac->_validateChecksum->setChecked(_cfg->readBoolEntry("Validate Checksum", false));
      _ac->_curtailRawData->setChecked(_cfg->readBoolEntry("Raw Data Curtail To Buffer", false));
      _ac->_rawDataBufferSize->setText(str.setNum(_cfg->readNumEntry("Raw Data Buffer Size", RAW_DATA_BUFFER_SIZE), 10));

      if (hasInstance) {
        KstSharedPtr<ScubaSource> src = kst_cast<ScubaSource>(_instance);
        if (src) {
          _cfg->setGroup(src->fileName());
          _ac->_readMatrices->setChecked(_cfg->readBoolEntry("Read Matrices", _ac->_readMatrices->isChecked()));
          _ac->_validateChecksum->setChecked(_cfg->readBoolEntry("Validate Checksum", _ac->_validateChecksum->isChecked()));
          _ac->_curtailRawData->setChecked(_cfg->readBoolEntry("Raw Data Curtail To Buffer", _ac->_curtailRawData->isChecked()));
          _ac->_rawDataBufferSize->setText(str.setNum(_cfg->readNumEntry("Raw Data Buffer Size", _ac->_rawDataBufferSize->text().toInt()), 10));
        }
      }
    }

    virtual void save() {
      assert(_cfg);
      _cfg->setGroup("SCUBA General");

      KstSharedPtr<ScubaSource> src = kst_cast<ScubaSource>(_instance);
      if (src) {
        _cfg->setGroup(src->fileName());
      }
      _cfg->writeEntry("Read Matrices", _ac->_readMatrices->isChecked());
      _cfg->writeEntry("Validate Checksum", _ac->_validateChecksum->isChecked());
      _cfg->writeEntry("Raw Data Curtail To Buffer", _ac->_curtailRawData->isChecked());
      _cfg->writeEntry("Raw Data Buffer Size", _ac->_rawDataBufferSize->text().toInt());

      //
      // update the instance from our new settings...
      //
      if (src && src->reusable()) {
        src->_config->read(_cfg, src->fileName());
      }
    }

    ScubaConfig *_ac;
};


extern "C" {
  KstDataSource *create_scuba(KConfig *cfg, const QString& filename, const QString& type) {
    return new ScubaSource(cfg, filename, type);
  }

  KstDataSource *load_scuba(KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e) {
    return new ScubaSource(cfg, filename, type, e);
  }

  QStringList provides_scuba() {
    QStringList rc;
    rc += "SCUBA";
    return rc;
  }

  int understands_scuba(KConfig *cfg, const QString& filename) {
    ScubaSource::Config config;
    config.read(cfg, filename);
    int retVal = 0;

    if (QFile::exists(filename) && !QFileInfo(filename).isDir()) {
      QFile f(filename);
      QString runFilename;

      //
      // check for the presence of a .run file
      //
      runFilename = ScubaSource::runFile(filename);
      if (!runFilename.isEmpty()) {
        QFile frun(runFilename);
        QString s;

        if (frun.open(IO_ReadOnly)) {
          //
          // we are not guaranteed that the HEADER tag is the first entry so
          //  search the entire file for it...
          //
          while (frun.readLine(s, 10) >= 0) {
            if (s.stripWhiteSpace().upper().compare(BEGIN_HEADER_2) == 0) {
              retVal = 100;

              break;
            }
          }
          frun.close();
        }
      } else if (f.open(IO_ReadOnly)) {
        QString s;
        Q_LONG rc = 0;
        bool done = false;
        int rownum = 0;

        while (!done && rownum < 2000) {
          rc = f.readLine(s, 1000);
          if (rc < 0) {
            retVal = 0;
            done = true;
          } else if (s.compare(END_HEADER_1) == 0) {
            retVal = 100;
            done = true;
          }
          ++rownum;
        }

        f.close( );
      } else {
        retVal = 0;
      }
    }

    return retVal;
  }


  QStringList fieldList_scuba(KConfig *cfg, const QString& filename, const QString& type, QString *typeSuggestion, bool *complete) {
    if ((!type.isEmpty() && !provides_scuba().contains(type)) ||
        0 == understands_scuba(cfg, filename)) {
      if (complete) {
        *complete = false;
      }
      return QStringList();
    }

    if (typeSuggestion) {
      *typeSuggestion = "SCUBA";
    }

    ScubaSource::Config config;

    config.read(cfg, filename);
    QStringList rc = ScubaSource::fieldListFor(filename, &config);

    if (complete) {
      *complete = rc.count() > 1;
    }

    return rc;
  }


  QWidget *widget_scuba(const QString& filename) {
    Q_UNUSED(filename)
    return new ConfigWidgetScuba;
  }
}

KST_KEY_DATASOURCE_PLUGIN(scuba)

