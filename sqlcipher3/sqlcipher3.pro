#-------------------------------------------------
#
# Project created by QtCreator 2013-03-18T18:02:30
#
#-------------------------------------------------

QT       -= sql -core

QT       -= gui

win32:CONFIG(release, debug|release): TARGET = sqlcipher
else:win32:CONFIG(debug, debug|release): TARGET = sqlcipherd
TEMPLATE = lib
CONFIG += staticlib \
        release
DESTDIR =$$PWD/lib


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

HEADERS += \
    sqlite3ext.h \
    sqlite3.h

SOURCES += \
    sqlite3.c

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -llibeay32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -llibeay32d

INCLUDEPATH += $$PWD/include \
        $$PWD/include/sqlchiper
DEPENDPATH += $$PWD/include

#win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/libeay32.lib
#else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/libeay32d.lib
