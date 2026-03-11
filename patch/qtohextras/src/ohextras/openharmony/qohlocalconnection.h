
/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#ifndef QOHLOCALCONNECTION_H
#define QOHLOCALCONNECTION_H

#include <QtOhExtras/qopenharmonyextrasglobal.h>
#include <QtOhExtras/qohabstractlocalsocket.h>

QT_BEGIN_NAMESPACE
class QOhLocalConnectionPrivate;
class QOhLocalServerPrivate;
class QJsObject;
class Q_OPENHARMONYEXTRAS_EXPORT QOhLocalConnection : public QOhAbstractLocalSocket
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QOhLocalConnection)
    friend class QOhLocalServerPrivate;
public:
    explicit QOhLocalConnection(QObject *parent = nullptr);

private:
    QOhLocalConnection(QJsObject *connection);
};

QT_END_NAMESPACE

#endif
