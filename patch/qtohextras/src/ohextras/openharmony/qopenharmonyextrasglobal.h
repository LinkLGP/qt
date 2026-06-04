/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#ifndef QOPENHARMONYEXTRASGLOBAL_H
#define QOPENHARMONYEXTRASGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#if defined(QT_BUILD_OPENHARMONYEXTRAS_LIB)
#define Q_OPENHARMONYEXTRAS_EXPORT Q_DECL_EXPORT
#else
#define Q_OPENHARMONYEXTRAS_EXPORT Q_DECL_IMPORT
#endif
#else
#define Q_OPENHARMONYEXTRAS_EXPORT
#endif

QT_END_NAMESPACE


#endif \
    // QOPENHARMONYEXTRASGLOBAL_H
