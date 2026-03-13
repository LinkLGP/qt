
/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#ifndef QOHLOCALSOCKET_H
#define QOHLOCALSOCKET_H

#include <QtOhExtras/qopenharmonyextrasglobal.h>
#include <QtOhExtras/qohabstractlocalsocket.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE
class QOhLocalSocketPrivate;

class Q_OPENHARMONYEXTRAS_EXPORT QOhLocalSocket : public QOhAbstractLocalSocket
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QOhLocalSocket)
public:
    explicit QOhLocalSocket(QObject *parent = nullptr);
    bool connect(const QString &address);
    void disconnectFromHost();
    bool isConnected() const;
};

QT_END_NAMESPACE

#endif
