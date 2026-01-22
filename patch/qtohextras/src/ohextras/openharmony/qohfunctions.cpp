/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * 文件名称: qohfunctions.cpp
 * 简要描述: 提供鸿蒙Native开发的辅助接口函数
 * 创建日期: 2024/11/06
 * 作者: WangHao
 * 说明:
 *
 * 修改日期: 2025/1/14
 * 作者: WangHao
 * 说明: 添加接口
 * requestPermissionsOnSetting
 * requestPermissionsOnSettingSync
 * **************************************************************** */
#include "qohfunctions.h"

#include <QDebug>
#include <QJsModule>
#include <qopenharmony.h>

#include "qpa/qplatformaccessctrl.h"
#include "qpa/qplatformintegration.h"
#include "private/qguiapplication_p.h"

QT_BEGIN_NAMESPACE

namespace QtOh {
static PermissionRequestResult privateToPublicPermissionsResult(const QtOhPrivate::PermissionRequestResult &result)
{
    QList<PermissionResult> convertedAuthResults;
    for (const auto &authResult : result.authResults) {
        switch (authResult) {
            case QtOhPrivate::PermissionsResult::Denied:
                convertedAuthResults.append(PermissionResult::Denied);
                break;
            case QtOhPrivate::PermissionsResult::Granted:
                convertedAuthResults.append(PermissionResult::Granted);
                break;
            case QtOhPrivate::PermissionsResult::InValidRequest:
                convertedAuthResults.append(PermissionResult::InValidRequest);
                break;
            default:
                convertedAuthResults.append(PermissionResult::InValidRequest);
                break;
        }
    }

    return QtOh::PermissionRequestResult(result.permissions, convertedAuthResults, result.dialogShownResults);
}
#undef ENABLE_SYS_MODULE
int ohSdkVersion()
{
#if defined(ENABLE_SYS_MODULE)
    /* @hms.core.atomicserviceComponent.atomicservice还能获取主题等信息 */
    int version = -1;
    QJsModule sysModule("@hms.core.atomicserviceComponent.atomicservice");
    Napi::Array sysInfoTypes = Napi::Array::New(sysModule.env());
    sysInfoTypes.Set(uint32_t(0), std::string("sdkApiVersion"));

    Napi::Object info = sysModule.call("getSystemInfoSync", { sysInfoTypes }).As<Napi::Object>();
    if (!info.IsNull()) {
        version = info.Get("sdkApiVersion").ToNumber();
    }
    return version;
#endif
    return QtOh::apiVersion();
}

PermissionResult checkPermission(const QString &permission)
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return PermissionResult{ -1 };
    }
    int result = accessCtrl->checkPermission(permission);
    return PermissionResult{ result };
}

PermissionRequestResult requestPermissionsSync(const QStringList &permissions, int timeoutMs)
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return PermissionRequestResult();
    }

    QtOhPrivate::PermissionRequestResult privateResult = accessCtrl->requestPermissionsSync(permissions, timeoutMs);
    return privateToPublicPermissionsResult(privateResult);
}

void requestPermissions(const QStringList &permissions, const PermissionResultCallback &callbackFunc)
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return;
    }

    accessCtrl->requestPermissions(permissions, [callbackFunc](const QtOhPrivate::PermissionRequestResult &result) {
        callbackFunc(privateToPublicPermissionsResult(result));
    });
}

PermissionRequestResult requestPermissionsOnSettingSync(const QStringList &permissions, int timeoutMs)
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return PermissionRequestResult();
    }

    QtOhPrivate::PermissionRequestResult privateResult =
        accessCtrl->requestPermissionsOnSettingSync(permissions, timeoutMs);
    return privateToPublicPermissionsResult(privateResult);
}

void requestPermissionsOnSetting(const QStringList &permissions, const PermissionResultCallback &callbackFunc)
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return;
    }

    accessCtrl->requestPermissionsOnSetting(permissions,
        [callbackFunc](const QtOhPrivate::PermissionRequestResult &result) {
            callbackFunc(privateToPublicPermissionsResult(result));
        });
}

void requestEnableNotification(const enableNotificationFunc &callbackFunc)
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return;
    }

    accessCtrl->requestEnableNotification([callbackFunc](bool result) { callbackFunc(result); });
}

bool requestEnableNotificationSync(int timeoutMs)
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return false;
    }

    return accessCtrl->requestEnableNotificationSync(timeoutMs);
}

bool isNotificationEnabled()
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return false;
    }

    return accessCtrl->isNotificationEnabled();
}

void openNotificationSettings()
{
    QPlatformAccessCtrl *accessCtrl = QGuiApplicationPrivate::platformIntegration()->accessCtrl();
    if (nullptr == accessCtrl) {
        qWarning() << Q_FUNC_INFO << "get QPlatformAccessCtrl failed.";
        return;
    }

    accessCtrl->openNotificationSettings();
}

}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator << (QDebug dbg, const QtOh::PermissionRequestResult &result)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg.noquote();

    dbg << "PermissionRequestResult(";
    dbg << "permissions: " << result.permissions << ", ";
    dbg << "authResults: [";
    for (int i = 0; i < result.authResults.size(); ++i) {
        if (i > 0)
            dbg << ", ";
        switch (result.authResults[i]) {
            case QtOh::PermissionResult::Denied:
                dbg << "Denied";
                break;
            case QtOh::PermissionResult::Granted:
                dbg << "Granted";
                break;
            case QtOh::PermissionResult::InValidRequest:
                dbg << "InValidRequest";
                break;
            default:
                dbg << "Unknown";
                break;
        }
    }
    dbg << "], ";
    dbg << "dialogShownResults: " << result.dialogShownResults; /* 打印 dialogShownResults */
    dbg << ")";

    return dbg;
}
#endif

QT_END_NAMESPACE
