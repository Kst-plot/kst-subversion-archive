include($$PWD/../../../config.pri)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    despike \
    cumulativesum \
    differentiation

LibExists(gsl) {
     SUBDIRS += butterworth_bandpass \
                butterworth_bandstop \
                butterworth_highpass \
                butterworth_lowpass
 }
