#ifndef QMOSQUITTO_GLOBAL_H
#define QMOSQUITTO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QMOSQUITTO_LIBRARY)
#  define QMOSQUITTOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QMOSQUITTOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QMOSQUITTO_GLOBAL_H
