/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include "qohprocess.h"
#include <private/qobject_p.h>
#include <qopenharmonyability.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qopenharmony.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qnapi.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qdebug.h>
#include <QtCore/quuid.h>
#include <QtCore/qtimer.h>
#include "qohlocalserver.h"
#include "qohlocalsocket.h"
#include "qohlocalconnection.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
QT_BEGIN_NAMESPACE

#define CHILD_TIMER_INTERVAL_MS 1000
#define SIGNALED_EXIT_OFFSET 128
namespace {
static QString PID = QString::fromUtf8("pid:");
static QString STATE = QString::fromUtf8("state:");
static QString QUIT = QString::fromUtf8("quit");
}

static void handleServerNewConnection(QOhLocalServer *server, qint64 &processPid, QEventLoop &loop)
{
    auto c = server->nextPendingConnection();
    if (!c) {
        return;
    }
    QObject::connect(c,
                     &QOhLocalConnection::messageReady,
                     c,
                     [&](const QString &message) {
                         if (message.startsWith(PID)) {
                             int pidLen = message.length() - PID.length();
                             QString pid = message.right(pidLen);
                             processPid = pid.toLongLong();
                             if (loop.isRunning()) {
                                 loop.quit();
                             }
                         }
                     });
}

static void setupServerConnection(QOhLocalServer *server, qint64 &processPid, QEventLoop &loop)
{
    QObject::connect(server, &QOhLocalServer::newConnection, server,
                     [server, &loop, &processPid] {
                         handleServerNewConnection(server, processPid, loop);
                     });
}

class MainProcessHandler
{
public:
    explicit MainProcessHandler(QOhProcessPrivate *p) : m_p(p) {}
    ~MainProcessHandler();
    void listen();
    void readMessage(const QString &message);
    void sendMessage(const QString &message);
private:
    QScopedPointer<QOhLocalServer> m_server;
    QOhProcessPrivate *m_p = nullptr;
    QOhLocalConnection* m_childConnection = nullptr;
};

MainProcessHandler::~MainProcessHandler()
{
    if (!m_server.isNull())
        m_server->close();
}

void MainProcessHandler::sendMessage(const QString &message)
{
    if (m_childConnection == nullptr) {
        return;
    }
    m_childConnection->send(message);
}

class ProcessHandler
{
public:
    explicit ProcessHandler(QCoreApplication *app) : m_app(app) {}
    ~ProcessHandler();
    void sendPid();
    void sendMessage(const QString &message);
    void readMessage(const QString &message);
    QOhLocalSocket *socket();
private:
    QCoreApplication *m_app = nullptr;
    QScopedPointer<QOhLocalSocket> m_socket;
};

ProcessHandler::~ProcessHandler()
{
    if (m_socket.isNull()) {
        return;
    }
    m_socket->close();
}

void ProcessHandler::sendPid()
{
    if (m_app == nullptr) {
        return;
    }
    auto pid = m_app->applicationPid();
    QString message = QString("%1%2").arg(PID).arg(pid);
    sendMessage(message);
}

void ProcessHandler::sendMessage(const QString &message)
{
    auto s = socket();
    if (s == nullptr) {
        return;
    }
    s->send(message);
}

void ProcessHandler::readMessage(const QString &message)
{
    if (message == QUIT) {
        if (qApp)
            qApp->quit();
    }
}

QOhLocalSocket *ProcessHandler::socket()
{
    if (!m_socket.isNull())
        m_socket.data();
    QString appName = m_app->applicationName();
    QString targetServerName = appName.split(".").first();
    auto socket = new QOhLocalSocket();
    if (socket->connect(targetServerName)) {
        QObject::connect(socket, &QOhLocalSocket::messageReady, socket, [this](const QString &message) {
            readMessage(message);
        });
        m_socket.reset(socket);
    } else {
        qWarning() << "create local socket failed: " << socket->errorString();
        delete socket;
        socket = nullptr;
    }
    return socket;
}

class QOhProcessResultReceiver : public QOpenHarmonyAbility::QAbilityResultReceiver
{
public:
    explicit QOhProcessResultReceiver(QOhProcessPrivate *p);
    virtual void handleResult(const Napi::Value &err, const Napi::Value &result = Napi::Value());
private:
    QOhProcessPrivate *m_private;
};

class QOhProcessPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QOhProcess)
public:
    QOhProcessPrivate();
    QString m_program;
    QStringList m_arguments;
    QString m_errorString;
    QOhProcess::Mode m_mode = QOhProcess::NEW_PROCESS_ATTACH_TO_PARENT;
    QScopedPointer<QOhProcessResultReceiver> m_receiver;
    QScopedPointer<ProcessHandler> m_processHandler;
    QScopedPointer<MainProcessHandler> m_mainHandler;
    qint64 m_pid = -1;
    int m_exitCode = 0;
    int m_childTimerId = 0;
    bool m_crashed = false;
    bool m_isRunning = false;
    static QStringList m_all;

    void startChild();
    void checkChildProcessState(int option = WNOHANG);

    void stop();
    void started();
    void stopped();
    void handleError(const QString &errorString);
    void waitForFinished();
    void startTimer();
    void killTimer();
    void setErrorAndEmit(QProcess::ProcessError error, const QString &errorString);
};

void MainProcessHandler::readMessage(const QString &message)
{
    if (message.startsWith(PID)) {
        QString pid = message.right(message.length() - PID.length());
        m_p->m_pid = pid.toLongLong();
        m_p->started();
    }
}

void MainProcessHandler::listen()
{
    if (m_server.isNull() && m_p != nullptr) {
        QScopedPointer<QOhLocalServer> server(new QOhLocalServer);
        QStringList name = m_p->m_program.split(".");
        if (!server->listen(name.first())) {
            qWarning() << "start server failed:" << server->errorString();
            return;
        }
        m_server.reset(server.take());
        QObject::connect(m_server.data(), &QOhLocalServer::newConnection, m_server.data(),
            [this] {
                m_childConnection = m_server->nextPendingConnection();
                if (!m_childConnection) {
                    return;
                }
                QObject::connect(m_childConnection,
                    &QOhLocalConnection::messageReady,
                    m_childConnection,
                    [this](const QString &message) {
                        readMessage(message);
                    });
            });
    }
}

QStringList QOhProcessPrivate::m_all;

QOhProcess::QOhProcess(QObject *parent)
    : QObject(*new QOhProcessPrivate, parent)
{
}

QOhProcess::QOhProcess(QCoreApplication *app, QObject *parent)
    : QObject(*new QOhProcessPrivate, parent)
{
    Q_D(QOhProcess);
    if (app != nullptr) {
        d->m_pid = app->applicationPid();
        d->m_processHandler.reset(new ProcessHandler(app));
        d->m_processHandler->sendPid();
    }
}

QOhProcess::~QOhProcess()
{
    stop();
    Q_D(QOhProcess);
    d->waitForFinished();
}

void QOhProcess::start(const QString &program, const QStringList &arguments)
{
    Q_D(QOhProcess);
    if (program.isEmpty()) {
        d->setErrorAndEmit(QProcess::FailedToStart, tr("No program defined"));
        return;
    }
    if (QOhProcessPrivate::m_all.contains(program))
        return;
    bool changed = !d->m_program.isEmpty() && (d->m_program != program);
    if (changed && d->m_isRunning) {
        stop();
        d->waitForFinished();
    }
    d->m_program = program;
    d->m_arguments = arguments;
    d->m_mainHandler.reset(new MainProcessHandler(d));
    d->m_mainHandler->listen();
    d->startChild();
}

qint64 QOhProcess::startDetached(const QString &program, const QString &moduleName,
                                 const QString &abilityName, const QString &bundleName)
{
    QOpenHarmonyWant want;
    want.bundleName = bundleName.isEmpty() ? QtOh::bundleName() : bundleName;
    want.moduleName = moduleName.isEmpty() ? QtOh::moduleName() : moduleName;
    want.parameters.insert("applicationName", program);
    QString processIdentifier = QString("process-" + QUuid::createUuid().toString(QUuid::WithoutBraces));
    want.parameters.insert("processIdentifier", processIdentifier);

    want.abilityName = abilityName.isEmpty() ? QString::fromUtf8("ChildEntryAbility") : abilityName;

    QScopedPointer<QOhLocalServer> server(new QOhLocalServer);
    QEventLoop loop;
    qint64 processPid = -1;
    QStringList name = program.split(".");
    if (!server->listen(name.first())) {
        qWarning() << "start server failed:" << server->errorString();
    } else {
        setupServerConnection(server.data(), processPid, loop);
    }
    QOpenHarmonyAbility::start(want);
    const int kMs = 2000;
    QTimer::singleShot(kMs, [&] {
        if (loop.isRunning())
            loop.quit();
    });
    loop.exec();
    return processPid;
}

void QOhProcess::setMode(Mode mode)
{
    Q_D(QOhProcess);
    d->m_mode = mode;
}

void QOhProcess::stop()
{
    Q_D(QOhProcess);
    if (!d->m_isRunning)
        return;
    d->stop();
}

bool QOhProcess::isRunning() const
{
    Q_D(const QOhProcess);
    return d->m_isRunning;
}

QString QOhProcess::errorString() const
{
    Q_D(const QOhProcess);
    return d->m_errorString;
}

qint64 QOhProcess::pid() const
{
    Q_D(const QOhProcess);
    return d->m_pid;
}

void QOhProcess::timerEvent(QTimerEvent *event)
{
    Q_D(QOhProcess);
    if (event->timerId() == d->m_childTimerId) {
        d->checkChildProcessState();
    }
    QObject::timerEvent(event);
}

QOhProcessPrivate::QOhProcessPrivate()
{
    m_receiver.reset(new QOhProcessResultReceiver(this));
}

void QOhProcessPrivate::startChild()
{
    QOpenHarmonyWant want;
    want.bundleName = QtOh::bundleName();
    want.moduleName = QtOh::moduleName();
    want.parameters.insert("applicationName", m_program);
    QString processIdentifier = QString("process-" + QUuid::createUuid().toString(QUuid::WithoutBraces));
    want.parameters.insert("processIdentifier", processIdentifier);

    want.abilityName = QString("ChildEntryAbility");

    QOpenHarmonyStartOptions startOptions;
    startOptions.processMode = static_cast<int>(m_mode);
    // 是否显示Ability，0不显示，1显示
    startOptions.startupVisibility = 1;
    Q_Q(QOhProcess);
    emit q->stateChanged(QProcess::Starting, QOhProcess::QPrivateSignal());
    QOpenHarmonyAbility::start(want, startOptions, m_receiver.data());
}

void QOhProcessPrivate::checkChildProcessState(int option)
{
    int status = 0;
    pid_t result = waitpid(m_pid, &status, option);
    if (result == -1) {
        qWarning() << QString("waitpid for: %1 failed,").arg(m_pid) << "error: " << errno;
        return;
    } else if (result == 0) {
        // running
    } else {
        m_isRunning = false;
        if (WIFEXITED(status)) {
            m_exitCode = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            m_crashed = true;
            m_exitCode = SIGNALED_EXIT_OFFSET + WTERMSIG(status);
        }
    }
    if (!m_isRunning) {
        if (m_childTimerId != 0) {
            Q_Q(QOhProcess);
            q->killTimer(m_childTimerId);
        }
        stopped();
    }
}

void QOhProcessPrivate::stop()
{
    m_mainHandler->sendMessage(QUIT);
}

void QOhProcessPrivate::started()
{
    Q_Q(QOhProcess);
    QOhProcessPrivate::m_all << m_program;
    startTimer();
    m_isRunning = true;
    emit q->stateChanged(QProcess::Running, QOhProcess::QPrivateSignal());
    emit q->started(QOhProcess::QPrivateSignal());
}

void QOhProcessPrivate::stopped()
{
    Q_Q(QOhProcess);
    QOhProcessPrivate::m_all.removeOne(m_program);
    m_pid = -1;
    m_mainHandler.reset();
    emit q->stateChanged(QProcess::NotRunning, QOhProcess::QPrivateSignal());
    emit q->finished(m_exitCode, m_crashed ? QProcess::CrashExit : QProcess::NormalExit);
    if (m_crashed) {
        emit q->errorOccurred(QProcess::Crashed);
    }
}

void QOhProcessPrivate::handleError(const QString &errorString)
{
    m_errorString = errorString;
    Q_Q(QOhProcess);
    emit q->errorOccurred(QProcess::FailedToStart);
}

void QOhProcessPrivate::waitForFinished()
{
    killTimer();
    checkChildProcessState(0);
}

void QOhProcessPrivate::startTimer()
{
    if (m_childTimerId == 0) {
        Q_Q(QOhProcess);
        m_childTimerId = q->startTimer(CHILD_TIMER_INTERVAL_MS);
    }
}

void QOhProcessPrivate::killTimer()
{
    if (m_childTimerId != 0) {
        Q_Q(QOhProcess);
        q->killTimer(m_childTimerId);
        m_childTimerId = 0;
    }
}

void QOhProcessPrivate::setErrorAndEmit(QProcess::ProcessError error, const QString &errorString)
{
    m_errorString = errorString;
    Q_Q(QOhProcess);
    emit q->errorOccurred(error);
}

QOhProcessResultReceiver::QOhProcessResultReceiver(QOhProcessPrivate *p)
    : m_private(p)
{
}

void QOhProcessResultReceiver::handleResult(const Napi::Value &err, const Napi::Value &result)
{
    int code = QNapi::get<int>(err, "code");
    if (code != 0) {
        if (m_private != nullptr)
            m_private->handleError(QNapi::get<QString>(err, "message"));
    }
}

QT_END_NAMESPACE
