#ifndef ALSON_API_GLOBAL_H
#define ALSON_API_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ALSON_API_LIBRARY)
#  define ALSON_API_EXPORT Q_DECL_EXPORT
#else
#  define ALSON_API_EXPORT Q_DECL_IMPORT
#endif

#endif // ALSON_API_GLOBAL_H
