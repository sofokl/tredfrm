#
# Build SqlCipher as a SQL driver plugin for Qt 5
#
# Simon Knopp, Feb 2014
#

#SQLCIPHER_CONFIGURE = --enable-tempstore=yes \
#                      --disable-tcl \
#                      CFLAGS="-DSQLITE_HAS_CODEC" \
#                      LDFLAGS="-lcrypto"

#SQLITE_DEFINES = SQLITE_OMIT_LOAD_EXTENSION SQLITE_OMIT_COMPLETE \
#                 SQLITE_ENABLE_FTS3 SQLITE_ENABLE_FTS3_PARENTHESIS \
#                 SQLITE_ENABLE_RTREE

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

TARGET = qdriverq


isEmpty(QT_SRCDIR):QT_SRCDIR = $$[QT_INSTALL_PREFIX]/../Src/qtbase
!exists($$QT_SRCDIR/src) {
    message("Could not find Qt source in " $$QT_SRCDIR/src)
    error("You need to set QT_SRCDIR = /path/to/qt/source in .qmake.conf")
}

DRIVER_SRCDIR = $$QT_SRCDIR/src/sql/drivers/sqlite
PLUGIN_SRCDIR = $$QT_SRCDIR/src/plugins/sqldrivers

INCLUDEPATH += $$DRIVER_SRCDIR

SOURCES = smain.cpp
OTHER_FILES += qsqlcipher.json

# Use Qt's SQLite driver for most of the implementation
HEADERS += $$DRIVER_SRCDIR/qsql_sqlite_p.h
SOURCES += $$DRIVER_SRCDIR/qsql_sqlite.cpp

# Don't install in the system-wide plugins directory
CONFIG += force_independent
#CONFIG += staticlib

!system-sqlite:!contains(LIBS, .*sqlite3.*) {
    CONFIG(release, debug|release):DEFINES *= NDEBUG
    DEFINES += $$SQLITE_DEFINES
    !contains(CONFIG, largefile):DEFINES += SQLITE_DISABLE_LFS
    INCLUDEPATH += $$PWD/../sqlcipher3
    LIBS        += -L$$PWD/../sqlcipher3/lib -lsqlcipher -llibeay32
    QMAKE_RPATHDIR += $$PWD/../sqlcipher3/lib
} else {
    LIBS *= $$QT_LFLAGS_SQLITE
    QMAKE_CXXFLAGS *= $$QT_CFLAGS_SQLITE
}

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sqlchiper3/lib/ -lsqlchiper -llibeay32
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sqlchiper3/lib/ -lsqlchiperd -llibeay32
#else:unix: LIBS += -L$$PWD/sqlchiper3/lib/ -lsqlchiper

INCLUDEPATH += $$PWD/../sqlcipher3
DEPENDPATH += $$PWD/../sqlcipher3

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/sqlchiper3/lib/libsqlchiper.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/sqlchiper3/lib/libsqlchiperd.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/sqlchiper3/lib/sqlchiper.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/sqlchiper3/lib/sqlchiperd.lib
#else:unix: PRE_TARGETDEPS += $$PWD/sqlchiper3/lib/libsqlchiper.a

PLUGIN_CLASS_NAME = QSQLCipherDriverPlugin
QT  = core core-private sql-private

PLUGIN_TYPE = sqldrivers
load(qt_plugin)

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

DESTDIR = ../bin/sqldrivers

#include($$PLUGIN_SRCDIR/qsqldriverbase.pri)

# Configure sqlcipher
#config_sqlcipher.target = $$PWD/sqlcipher3/Makefile
#config_sqlcipher.commands = cd $$PWD/sqlcipher && \
#                            ./configure $$SQLCIPHER_CONFIGURE \
#                                --prefix=$$OUT_PWD

# Build sqlcipher
#sqlcipher.target = $$PWD/sqlcipher3/lib
#sqlcipher.commands = $(MAKE) -C $$PWD/sqlcipher3 install
#sqlcipher.depends = config_sqlcipher

# Configure and build sqlcipher before building the plugin
#QMAKE_EXTRA_TARGETS += config_sqlcipher sqlcipher
#PRE_TARGETDEPS += $$sqlcipher.target





