/* ***************************************************************************
 *
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include "qohchildprocess.h"
#include <QtCore/qjsobject.h>
#include <QtCore/qjsmodule.h>
#include <QtCore/qnapi.h>
#include <QtCore/qdebug.h>
#include <private/qopenharmony_p.h>
#include <private/qjspromise_p.h>

QT_BEGIN_NAMESPACE

qint64 QOhChildProcess::startChildProcess(const QString &srcEntry, StartMode mode)
{
    return QtOh::runOnJsUIThreadWithPromise<qint64>([srcEntry, mode](auto p) {
        static QJsModule childProcessManager("@ohos.app.ability.childProcessManager");
        auto result = childProcessManager.call("startChildProcess", srcEntry, mode);
        if (!result.IsPromise()) {
            p->set_value(-1);
            return;
        }
        QJsPromise promise(result.As<Napi::Promise>());
        promise.onThen([p](const Napi::CallbackInfo &info) {
                   p->set_value(QNapi::getFirst<qint64>(info));
               }).onCatch([p](const Napi::CallbackInfo &info) {
                    auto error = QNapi::getFirst<Napi::Object>(info);
                    qWarning() << "start child process failed: " << QNapi::get<QString>(error, "message");
                   p->set_value(-1);
                });
    });
}

static void freeFdList(NativeChildProcess_Fd* head)
{
    NativeChildProcess_Fd* current = head;
    while (current) {
        NativeChildProcess_Fd* next = current->next;
        if (current->fdName) {
            delete[] current->fdName;
        }
        delete current;
        current = next;
    }
}

static void freeProcessArgs(NativeChildProcess_Args& args)
{
    if (args.entryParams) {
        delete[] args.entryParams;
        args.entryParams = nullptr;
    }

    freeFdList(args.fdList.head);
    args.fdList.head = nullptr;
}

Ability_NativeChildProcess_ErrCode QOhChildProcess::startNativeChildProcess(const QString &entry,
                                                                            const Args &args,
                                                                            const Options &options,
                                                                            int32_t *pid)
{
    if (entry.isEmpty())
        return NCP_ERR_INVALID_PARAM;
    NativeChildProcess_Args ca;
    QByteArray entryParams = args.entryParams.toUtf8();
    ca.entryParams = new char[entryParams.size() + 1];
    strcpy(ca.entryParams, entryParams.constData());
    ca.fdList.head = nullptr;
    NativeChildProcess_Fd *tail = ca.fdList.head;
    for (int i = 0; i < args.fds.count(); ++i) {
        auto fd = args.fds.at(i);
        NativeChildProcess_Fd *newFd = new NativeChildProcess_Fd;
        newFd->fdName = new char[fd.name.length() + 1];
        strcpy(newFd->fdName, fd.name.toUtf8().constData());
        newFd->fd = fd.fd;
        newFd->next = nullptr;
        if (ca.fdList.head == nullptr) {
            ca.fdList.head = newFd;
            tail = newFd;
        } else {
            tail->next = newFd;
            tail = newFd;
        }
    }

    NativeChildProcess_Options co = {
        .isolationMode = static_cast<NativeChildProcess_IsolationMode>(options.isolationMode),
        .reserved = options.reserved
    };
    QByteArray entryData = entry.toUtf8();
    Ability_NativeChildProcess_ErrCode ret = OH_Ability_StartNativeChildProcess(
                   entryData.constData(), ca, co, pid);
    freeProcessArgs(ca);
    return ret;

}


int QOhChildProcess::createNativeChildProcess(const QString &entryPoint,
                                                OH_Ability_OnNativeChildProcessStarted onProcessStarted)
{

    QByteArray entry = entryPoint.toUtf8();
    return OH_Ability_CreateNativeChildProcess(entry.constData(), onProcessStarted);
}

#if OHOS_SDK_VERSION >= 20
int QOhChildProcess::createNativeChildProcessWithConfig(const QString &entryPoint,
                                                          const Config &config,
                                                          OH_Ability_OnNativeChildProcessStarted onProcessStarted)
{
    auto ohConfig = OH_Ability_CreateChildProcessConfigs();
    if (ohConfig == nullptr)
        return NCP_ERR_INTERNAL;
    OH_Ability_ChildProcessConfigs_SetIsolationMode(ohConfig, config.mode);
#if OHOS_SDK_VERSION >= 21
    OH_Ability_ChildProcessConfigs_SetIsolationUid(ohConfig, config.isolationUid);
#endif
    QByteArray data = config.processName.toUtf8();
    OH_Ability_ChildProcessConfigs_SetProcessName(ohConfig, data.constData());
    QByteArray entry = entryPoint.toUtf8();
    auto result = OH_Ability_CreateNativeChildProcessWithConfigs(entry, ohConfig, onProcessStarted);
    OH_Ability_DestroyChildProcessConfigs(ohConfig);
    return result;
}
#endif
QT_END_NAMESPACE

