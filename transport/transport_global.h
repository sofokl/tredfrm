#ifndef TRANSPORT_GLOBAL_H
#define TRANSPORT_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QDebug>

#if defined(TRANSPORT_LIBRARY)
#  define TRANSPORT_EXPORT Q_DECL_EXPORT
#else
#  define TRANSPORT_EXPORT Q_DECL_IMPORT
#endif

#endif // TRANSPORT_GLOBAL_H
