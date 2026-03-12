/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include "qohlocalserver.h"
#include <private/qobject_p.h>
#include <QtCore/qjsmodule.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qjsobject.h>
#include <private/qjspromise_p.h>
#include <QtCore/qopenharmony.h>
#include <private/qopenharmony_p.h>
#include <QtCore/qnapi.h>
#include "qohlocalconnection.h"
#include "qohabstractlocalsocket_p.h"

QT_BEGIN_NAMESPACE

class QOhLocalServerPrivate : public QOhAbstractLocalSocketPrivate
{
    Q_DECLARE_PUBLIC(QOhLocalServer)
public:
    QSharedPointer<QOhLocalConnection> m_connect;
    bool m_binded = false;

    bool bind(const QString &address);
    void onConnect();
};

QOhLocalServer::QOhLocalServer(QObject *parent)
    : QOhAbstractLocalSocket(*new QOhLocalServerPrivate, parent)
{
    QtOh::runOnJsUIThreadAndWait([this] {
        Q_D(QOhLocalServer);
        QJsModule socket("@ohos.net.socket");
        auto localServer = socket.call("constructLocalSocketServerInstance");
        d->m_object.reset(new QJsObject(localServer.As<Napi::Object>()));
    });
}

bool QOhLocalServer::listen(const QString &address)
{
    Q_D(QOhLocalServer);
    if (d->m_binded)
        return true;
    bool result = d->bind(address);
    if (!result)
        return false;
    d->onConnect();
    return true;
}

QOhLocalConnection *QOhLocalServer::nextPendingConnection() const
{
    Q_D(const QOhLocalServer);
    return d->m_connect.data();
}

void QOhLocalServer::close()
{
    QtOh::runOnJsUIThreadAndWait([this] {
        Q_D(QOhLocalServer);
        d->m_object->call("off", "connect");
    });
}

bool QOhLocalServerPrivate::bind(const QString &address)
{
    return QtOh::runOnJsUIThreadWithPromise<bool>([this, address](auto p) {
        auto option = QNapi::createObject();
        QString dir = QtOh::dir("DIRECTORY_HAP_FILES");
        QNapi::set(option, "address", QString("%1/%2").arg(dir).arg(address));
        QJsPromise promise(m_object->call("listen", option).As<Napi::Promise>());
        promise.onThen([p](const Napi::CallbackInfo &info) {
                   Q_UNUSED(info)
                   p->set_value(true);
               }).onCatch([this, p](const Napi::CallbackInfo &info) {
                    auto error = QNapi::getFirst<Napi::Object>(info);
                    m_errorString = QNapi::get<QString>(error, "message");
                    p->set_value(false);
                });
    });
}

void QOhLocalServerPrivate::onConnect()
{
    QtOh::runOnJsUIThreadAndWait([this] {
        auto callback = QNapi::create([this](const Napi::CallbackInfo &info) {
            Q_Q(QOhLocalServer);
            auto connect = QNapi::getFirst<Napi::Object>(info);
            m_connect.reset(new QOhLocalConnection(new QJsObject(connect)));
            emit q->newConnection();
        });
        m_object->call("on", "connect", callback);
    });
}
QT_END_NAMESPACE

