QT  += core sql widgets
QT  -= webkit angle

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

TARGET = Tredifarm
DESTDIR = ../bin/

VER_MAJ = 1
VER_MIN = 2
#VER_PAT = 14
VERSION = 1.2.0

BUILDN = 202

DEFINES += VER_MAJ=$$VER_MAJ
DEFINES += VER_MIN=$$VER_MIN
DEFINES += BUILDN=$$BUILDN
DEFINES += VERSION=$$VERSION

RC_FILE = app.rc

SOURCES += \
    main.cpp \
    FarmApp.cpp \
    settingsdialog.cpp \
    uploaddialog.cpp \
    diskid32.cpp \
    providersdialog.cpp

HEADERS += \
    FarmApp.h \
    settingsdialog.h \
    uploaddialog.h \
    diskid32.h \
    providersdialog.h

RESOURCES += \
    resources.qrc

FORMS += \
    settingsdialog.ui \
    uploaddialog.ui \
    MainWindow.ui \
    providersdialog.ui \
    providersdialog.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../bin/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../bin/ -lcored
else:unix: LIBS += -L$$OUT_PWD/../bin/ -lcore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../bin/ -ltransport
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../bin/ -ltransportd
else:unix: LIBS += -L$$OUT_PWD/../bin/ -ltransport

INCLUDEPATH += $$PWD/../transport
DEPENDPATH += $$PWD/../transport

OTHER_FILES += \
    app.rc


