
/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#ifndef QOHABSTRACTLOCALSOCKET_H
#define QOHABSTRACTLOCALSOCKET_H

#include <QtOhExtras/qopenharmonyextrasglobal.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE
class QOhAbstractLocalSocketPrivate;

class Q_OPENHARMONYEXTRAS_EXPORT QOhAbstractLocalSocket : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QOhAbstractLocalSocket)
public:
    explicit QOhAbstractLocalSocket(QObject *parent = nullptr);
    virtual ~QOhAbstractLocalSocket();
    bool send(const QString &message);
    QString errorString() const;

    virtual void close();
signals:
    void messageReady(const QString &message);

protected:
    QOhAbstractLocalSocket(QOhAbstractLocalSocketPrivate &d, QObject *parent);
};

QT_END_NAMESPACE

#endif
