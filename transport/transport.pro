QT       += core xml network widgets
DESTDIR = ../bin/

TEMPLATE = lib

win32:CONFIG(release, debug|release): TARGET = transport
else:win32:CONFIG(debug, debug|release): TARGET = transportd
else:unix: TARGET = transport


DEFINES += TRANSPORT_LIBRARY

SOURCES += \ 
    wsdl_service.cpp \
    KDSoapClient/KDSoapValue.cpp \
    KDSoapClient/KDSoapNamespaceManager.cpp \
    KDSoapClient/KDDateTime.cpp \
    KDSoapClient/KDSoapJob.cpp \
    KDSoapClient/KDSoapMessage.cpp \
    KDSoapClient/KDSoapPendingCallWatcher.cpp \
    KDSoapClient/KDSoapPendingCall.cpp \
    KDSoapClient/KDSoapClientInterface.cpp \
    KDSoapClient/KDSoapNamespacePrefixes.cpp \
    KDSoapClient/KDSoapMessageReader.cpp \
    KDSoapClient/KDSoapClientThread.cpp \
    KDSoapClient/KDSoapAuthentication.cpp \
    KDSoapClient/KDSoapSslHandler.cpp \
    KDSoapClient/KDSoapMessageWriter.cpp \
    transportproxy.cpp

HEADERS  += \
    wsdl_service.h \
    KDSoapClient/KDSoapValue.h \
    KDSoapClient/KDSoapNamespaceManager.h \
    KDSoapClient/KDDateTime.h \
    KDSoapClient/KDSoapJob.h \
    KDSoapClient/KDSoapMessage.h \
    KDSoapClient/KDSoapPendingCallWatcher.h \
    KDSoapClient/KDSoapPendingCall.h \
    KDSoapClient/KDSoapClientInterface.h \
    KDSoapClient/KDSoapPendingCallWatcher_p.h \
    KDSoapClient/KDSoapClientInterface_p.h \
    KDSoapClient/KDSoapNamespacePrefixes_p.h \
    KDSoapClient/KDSoapMessageReader_p.h \
    KDSoapClient/KDSoapClientThread_p.h \
    KDSoapClient/KDSoapAuthentication.h \
    KDSoapClient/KDSoapSslHandler.h \
    KDSoapClient/KDSoapMessageWriter_p.h \
    transport_global.h \
    transportproxy.h \
    KDSoapClient/KDSoapGlobal.h
