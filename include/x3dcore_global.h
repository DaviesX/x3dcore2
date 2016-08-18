#ifndef X3DCORE_GLOBAL_H
#define X3DCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(X3DCORE_LIBRARY)
#  define X3DCORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define X3DCORESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // X3DCORE_GLOBAL_H
