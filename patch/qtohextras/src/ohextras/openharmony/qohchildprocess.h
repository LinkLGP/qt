
/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#ifndef QOHCHILDPROCESS_H
#define QOHCHILDPROCESS_H

#include <QtOhExtras/qopenharmonyextrasglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <AbilityKit/native_child_process.h>

QT_BEGIN_NAMESPACE

namespace QOhChildProcess {

enum class StartMode {
    SELF_FORK,
    APP_SPAWN_FORK
};

// for childProcessManager.startChildProcess
Q_OPENHARMONYEXTRAS_EXPORT qint64 startChildProcess(const QString &srcEntry, StartMode mode);

enum class IsolationMode {
    NCP_ISOLATION_MODE_NORMAL,
    NCP_ISOLATION_MODE_ISOLATED
};

struct Options
{
    IsolationMode isolationMode;
    int64_t reserved;
};

struct Fd
{
    QString name;
    int32_t fd;
};

struct Args
{
    QString entryParams;
    QList<Fd> fds;
};


// for native API OH_Ability_StartNativeChildProcess
Q_OPENHARMONYEXTRAS_EXPORT Ability_NativeChildProcess_ErrCode
startNativeChildProcess(const QString &entry, const Args &args,
                        const Options &options, int32_t *pid);

typedef void (*OH_Ability_OnNativeChildProcessStarted)(int errCode, OHIPCRemoteProxy *remoteProxy);

//for native API OH_Ability_CreateNativeChildProcess
Q_OPENHARMONYEXTRAS_EXPORT int
createNativeChildProcess(const QString &entryPoint,
                        OH_Ability_OnNativeChildProcessStarted onProcessStarted);

#if OHOS_SDK_VERSION >= 20
struct ChildProcessConfig
{
    QString processName;
    IsolationMode mode;
#if OHOS_SDK_VERSION >= 21
    bool isolationUid;
#endif
};

Q_OPENHARMONYEXTRAS_EXPORT int
createNativeChildProcessWithConfig(const QString &entryPoint,
                                  const Config &config,
                                  OH_Ability_OnNativeChildProcessStarted onProcessStarted);
#endif
}

QT_END_NAMESPACE

#endif
