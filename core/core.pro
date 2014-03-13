QT       += widgets network sql

TEMPLATE = lib

win32:CONFIG(release, debug|release): TARGET = core
else:win32:CONFIG(debug, debug|release): TARGET = cored
else:unix: TARGET = core



DESTDIR = ../bin/

DEFINES += CORE_LIBRARY

SOURCES += \
    LkSearchTableView.cpp \
    LkSettingsManager.cpp \
    LkStoreHelper.cpp \
    LkSpinBoxDelegate.cpp \
    LkUpdateHelper.cpp

HEADERS += \
    LkSqlTableModel.h \
    LkSearchTableView.h \
    LkCheckedDelegate.h \
    LkSettingsManager.h \
    LkStoreHelper.h \
    core_global.h \
    LkSpinBoxDelegate.h \
    LkUpdateHelper.h
