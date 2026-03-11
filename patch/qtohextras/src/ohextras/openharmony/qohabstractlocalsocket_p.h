/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#ifndef QOHABSTRACTLOCALSOCKET_P_H
#define QOHABSTRACTLOCALSOCKET_P_H

#include <QtOhExtras/qopenharmonyextrasglobal.h>
#include <QtOhExtras/qohabstractlocalsocket.h>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE
class QJsObject;
class Q_OPENHARMONYEXTRAS_EXPORT QOhAbstractLocalSocketPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QOhAbstractLocalSocket)
public:
    QOhAbstractLocalSocketPrivate();
    QString m_errorString;
    QScopedPointer<QJsObject> m_object;

    void close();
    void onError();
    void onMessage();

};

QT_END_NAMESPACE

#endif
