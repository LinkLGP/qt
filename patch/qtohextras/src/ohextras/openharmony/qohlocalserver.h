
/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#ifndef QOHLOCALSERVER_H
#define QOHLOCALSERVER_H

#include <QtOhExtras/qopenharmonyextrasglobal.h>
#include <QtOhExtras/qohabstractlocalsocket.h>

QT_BEGIN_NAMESPACE
class QOhLocalServerPrivate;
class QOhLocalConnection;
class Q_OPENHARMONYEXTRAS_EXPORT QOhLocalServer : public QOhAbstractLocalSocket
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QOhLocalServer)
public:
    explicit QOhLocalServer(QObject *parent = nullptr);
    bool listen(const QString &address);
    QOhLocalConnection *nextPendingConnection() const;
    virtual void close() override;
signals:
    void newConnection();
};

QT_END_NAMESPACE

#endif
