
/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#ifndef QOHPROCESS_H
#define QOHPROCESS_H

#include <QtOhExtras/qopenharmonyextrasglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/QProcess>

QT_BEGIN_NAMESPACE
class QOhProcessPrivate;
class QCoreApplication;
class Q_OPENHARMONYEXTRAS_EXPORT QOhProcess : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QOhProcess)
public:
    enum Mode {
        NEW_PROCESS_ATTACH_TO_PARENT = 1,
        NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM,
        ATTACH_TO_STATUS_BAR_ITEM
    };

    explicit QOhProcess(QObject *parent = nullptr);
    explicit QOhProcess(QCoreApplication *app, QObject *parent = nullptr);
    ~QOhProcess();

    void start(const QString &program, const QStringList &arguments = QStringList());
    static qint64 startDetached(const QString &program,
                              const QString &moduleName = QString(),
                              const QString &abilityName = QString(),
                              const QString &bundleName = QString());
    void setMode(Mode mode);
    void stop();
    bool isRunning() const;
    QString errorString() const;
    qint64 pid() const;

Q_SIGNALS:
    void started(QPrivateSignal);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void errorOccurred(QProcess::ProcessError error);
    void stateChanged(QProcess::ProcessState state, QPrivateSignal);

    // QObject interface
protected:
    virtual void timerEvent(QTimerEvent *event) override;
};

QT_END_NAMESPACE

#endif
