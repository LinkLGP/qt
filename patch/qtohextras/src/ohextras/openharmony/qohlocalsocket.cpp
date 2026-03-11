/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include "qohlocalsocket.h"
#include <private/qobject_p.h>
#include <QtCore/qjsmodule.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qjsobject.h>
#include <QtCore/qopenharmony.h>
#include <QtCore/qnapi.h>
#include <private/qjspromise_p.h>
#include <private/qopenharmony_p.h>
#include <private/qohabstractlocalsocket_p.h>
QT_BEGIN_NAMESPACE

class QOhLocalSocketPrivate : public QOhAbstractLocalSocketPrivate
{
    Q_DECLARE_PUBLIC(QOhLocalSocket)
public:
    bool connectToHost(const QString &address);
    bool m_connected = false;
};

QOhLocalSocket::QOhLocalSocket(QObject *parent)
    : QOhAbstractLocalSocket(*new QOhLocalSocketPrivate, parent)
{
    QtOh::runOnJsUIThreadAndWait([this] {
        Q_D(QOhLocalSocket);
        QJsModule socket("@ohos.net.socket");
        auto localSocket = socket.call("constructLocalSocketInstance");
        d->m_object.reset(new QJsObject(localSocket.As<Napi::Object>()));
    });
}

bool QOhLocalSocket::connect(const QString &address)
{
    Q_D(QOhLocalSocket);
    if (d->m_connected)
        return true;
    return d->connectToHost(address);
}

void QOhLocalSocket::disconnectFromHost()
{
    Q_D(QOhLocalSocket);
    if (!d->m_connected)
        return;
    d->close();
    d->m_connected = false;
}

bool QOhLocalSocket::isConnected() const
{
    Q_D(const QOhLocalSocket);
    return d->m_connected;
}

bool QOhLocalSocketPrivate::connectToHost(const QString &address)
{
    m_connected = QtOh::runOnJsUIThreadWithPromise<bool>([this, address](auto p) {
        auto option = QNapi::createObject();
        QString dir = QtOh::dir("DIRECTORY_HAP_FILES");
        QString hostAddress = QString("%1/%2").arg(dir).arg(address);
        auto hostAddressObject = QNapi::createObject();
        QNapi::set(hostAddressObject, "address", hostAddress);
        QNapi::set(option, "address", hostAddressObject);
        QJsPromise promise(m_object->call("connect", option).As<Napi::Promise>());
        promise.onThen([p](const Napi::CallbackInfo &info) {
                   Q_UNUSED(info)
                   p->set_value(true);
               }).onCatch([this, p](const Napi::CallbackInfo &info) {
                    auto error = QNapi::getFirst<Napi::Object>(info);
                    m_errorString = QNapi::get<QString>(error, "message");
                    p->set_value(false);
                });
    });
    if (m_connected) {
        onMessage();
        onError();
    }
    return m_connected;
}


QT_END_NAMESPACE
