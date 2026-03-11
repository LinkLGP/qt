/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include "qohabstractlocalsocket.h"
#include <QtCore/qjsmodule.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qjsobject.h>
#include <private/qjspromise_p.h>
#include <QtCore/qopenharmony.h>
#include <private/qopenharmony_p.h>
#include <QtCore/qnapi.h>
#include "qohabstractlocalsocket_p.h"
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

QOhAbstractLocalSocketPrivate::QOhAbstractLocalSocketPrivate() {}

void QOhAbstractLocalSocketPrivate::close()
{
    m_object->call("close");
    m_object->call("off", "error");
    m_object->call("off", "message");
}

void QOhAbstractLocalSocketPrivate::onError()
{
    QtOh::runOnJsUIThreadAndWait([this]{
        auto callback = QNapi::create([this](const Napi::CallbackInfo &info) {
            auto error = QNapi::getFirst<Napi::Object>(info);
            m_errorString = QNapi::get<QString>(error, "message");
            qWarning() << "ipc operation failed: " << m_errorString;
        });
        m_object->call("on", "error", callback);
    });
}

void QOhAbstractLocalSocketPrivate::onMessage()
{
    QtOh::runOnJsUIThreadAndWait([this] {
        auto callback = QNapi::create([this](const Napi::CallbackInfo &info) {
            auto messageObject = QNapi::getFirst<Napi::Object>(info);
            auto message = QNapi::get<Napi::Object>(messageObject, "message");
            if (message.IsArrayBuffer()) {
                Napi::ArrayBuffer buffer = message.As<Napi::ArrayBuffer>();
                char* data = static_cast<char*>(buffer.Data());
                size_t length = buffer.ByteLength();
                Q_Q(QOhAbstractLocalSocket);
                emit q->messageReady(QString::fromUtf8(data, static_cast<int>(length)));
            }
        });
        m_object->call("on", "message", callback);
    });
}

QOhAbstractLocalSocket::QOhAbstractLocalSocket(QObject *parent)
    : QObject(*new QOhAbstractLocalSocketPrivate, parent)
{

}

QOhAbstractLocalSocket::~QOhAbstractLocalSocket()
{
    close();
}

bool QOhAbstractLocalSocket::send(const QString &message)
{
    return QtOh::runOnJsUIThreadWithPromise<bool>([this, message](auto p) {
        Q_D(QOhAbstractLocalSocket);
        auto option = QNapi::createObject();
        QNapi::set(option, "data", message);
        QJsPromise promise(d->m_object->call("send", option).As<Napi::Promise>());
        promise.onThen([p](const Napi::CallbackInfo &info) {
                   Q_UNUSED(info)
                   p->set_value(true);
               }).onCatch([this, p, d](const Napi::CallbackInfo &info) {
                    auto error = QNapi::getFirst<Napi::Object>(info);
                    d->m_errorString = QNapi::get<QString>(error, "message");
                    p->set_value(false);
                });
    });
}

QOhAbstractLocalSocket::QOhAbstractLocalSocket(QOhAbstractLocalSocketPrivate &d, QObject *parent)
    : QObject(d, parent)
{

}

QString QOhAbstractLocalSocket::errorString() const
{
    Q_D(const QOhAbstractLocalSocket);
    return d->m_errorString;
}

void QOhAbstractLocalSocket::close()
{
    Q_D(QOhAbstractLocalSocket);
    d->close();
}

QT_END_NAMESPACE
