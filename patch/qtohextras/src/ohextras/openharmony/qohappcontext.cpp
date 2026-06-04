/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * 文件名称: qohappcontext.cpp
 * 简要描述: 主题相关处理类
 * 创建日期: 2025/06/18
 * 作者: WangHao
 * 说明:
 *
 * 修改日期:
 * 作者:
 * 说明:
 * **************************************************************** */
#include <bit>
#include <mutex>
#include <qdebug.h>
#include <qjsobject.h>
#include <QScopedPointer>
#include <private/qobject_p.h>
#include <qpa/qplatformtheme.h>
#include <qpa/qplatformnativeinterface.h>
#include <QtGui/private/qguiapplication_p.h>
#include "qohfunctions.h"
#include "qohappcontext.h"

namespace {
enum class OhConfigurationColorMode {
    COLOR_MODE_NOT_SET = -1, /* 表示未设置颜色模式 */
    COLOR_MODE_DARK = 0,     /* 表示深色模式 */
    COLOR_MODE_LIGHT = 1     /* 表示浅色模式 */
};

static QOhAppContext::ColorMode sysColorMode2ColorMode(OhConfigurationColorMode colorMode)
{
    switch (colorMode) {
        case ::OhConfigurationColorMode::COLOR_MODE_LIGHT:
            return QOhAppContext::ColorMode::Light;
        case ::OhConfigurationColorMode::COLOR_MODE_DARK:
            return QOhAppContext::ColorMode::Dark;
        case ::OhConfigurationColorMode::COLOR_MODE_NOT_SET:
            return QOhAppContext::ColorMode::Default;
    }

    qWarning("%s: got unknown ColorMode: %d", Q_FUNC_INFO, static_cast<int>(colorMode));
    return QOhAppContext::ColorMode::Default;
}
}

class QOhAppContextPrivate : public QObjectPrivate {
    Q_DECLARE_PUBLIC(QOhAppContext)
public:
    ~QOhAppContextPrivate();

protected:
    void initialized();
    bool darkColorModeActive() const;

private:
    // 获取并验证UIAbilityContext
    QJsObject* getValidUIAbilityContext();
    // 初始化系统配置信息
    void initializeSystemConfiguration(QJsObject* jsContext);
    // 设置环境变化监听器
    void setupEnvironmentListener(QJsObject* jsContext);
    /* 设置pcMode变化回调 */
    void registerPcModeChangedCallback();
    // 创建配置更新回调函数
    Napi::Function createConfigurationUpdateCallback(const Napi::Env& env);

    bool m_followSystemColorMode = false;
    Napi::FunctionReference m_callbackRef;
    inline static std::once_flag m_once{};
    inline static QScopedPointer<QOhAppContext> m_appContext{ nullptr };

    QOhAppContext::ColorMode m_colorMode = QOhAppContext::ColorMode::Default;
    OhConfigurationColorMode m_sysColorMode = OhConfigurationColorMode::COLOR_MODE_LIGHT;
};

QOhAppContextPrivate::~QOhAppContextPrivate()
{
    m_callbackRef.Reset();
}

QJsObject* QOhAppContextPrivate::getValidUIAbilityContext()
{
    void *context = QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("UIAbilityContext");
    if (context == nullptr) {
        qWarning() << "UIAbilityContext is null.";
        return nullptr;
    }

    QJsObject *jsContext = std::bit_cast<QJsObject *>(context);
    if (jsContext == nullptr) {
        qWarning() << Q_FUNC_INFO << "get uiabilitycontext failed.";
        return nullptr;
    }

    return jsContext;
}

void QOhAppContextPrivate::initializeSystemConfiguration(QJsObject* jsContext)
{
    Napi::Object config = jsContext->get("config").ToObject();
    if (!config.IsNull()) {
        m_sysColorMode = OhConfigurationColorMode(config.Get("colorMode").ToNumber().Int32Value());
    }
}

Napi::Function QOhAppContextPrivate::createConfigurationUpdateCallback(const Napi::Env& env)
{
    return Napi::Function::New(env, [this](const Napi::CallbackInfo &info) {
        if (0 >= info.Length())
            return;

        Napi::Object config = info[0].ToObject();
        Napi::Value value = config.Get("colorMode");
        OhConfigurationColorMode colorMode = OhConfigurationColorMode(value.ToNumber().Int32Value());
        
        if (m_followSystemColorMode) {
            QMetaObject::invokeMethod(
                qApp,
                [colorMode] { QGuiApplicationPrivate::platformTheme()->setThemeColorMode(int(colorMode)); },
                Qt::QueuedConnection);
        }
        if (m_sysColorMode != colorMode) {
            Q_Q(QOhAppContext);
            m_sysColorMode = colorMode;
            Q_EMIT q->sysThemChanged(OhConfigurationColorMode::COLOR_MODE_DARK == m_sysColorMode
                                     ? QOhAppContext::SysThem::Dark : QOhAppContext::SysThem::Light,
                              QOhAppContext::QPrivateSignal());
        }
    });
}

void QOhAppContextPrivate::setupEnvironmentListener(QJsObject* jsContext)
{
    Napi::Object environmentCallback = Napi::Object::New(jsContext->env());
    Napi::Function configurationUpdated = createConfigurationUpdateCallback(environmentCallback.Env());
    
    m_callbackRef = Napi::Persistent(configurationUpdated);
    environmentCallback.Set("onConfigurationUpdated", configurationUpdated);

    QJsObject appContext(jsContext->call("getApplicationContext").ToObject());
    appContext.call("on", { Napi::String::From(appContext.env(), "environment"), environmentCallback });
}

void QOhAppContextPrivate::registerPcModeChangedCallback()
{
    auto platformNativeInterface = qApp->platformNativeInterface();
    if (platformNativeInterface == nullptr) {
        return;
    }
    QJsObject *obj =
            reinterpret_cast<QJsObject *>(platformNativeInterface->nativeResourceForIntegration("nativeSettings"));

    if (!obj)
        return;
    Q_Q(QOhAppContext);
    Napi::Function pcModeChangedListener = Napi::Function::New(obj->env(),
                                               [guard = QPointer<QOhAppContext>(q)](const Napi::CallbackInfo& info) {
                                                   if (guard.isNull() || info.Length() < 1 || !info[0].IsBoolean())
                                                       return;
                                                   bool pcMode = info[0].ToBoolean();
                                                   emit guard->pcModeChanged(pcMode);
                                               });
    obj->object().Set("pcModeChangedListener", pcModeChangedListener);
}

void QOhAppContextPrivate::initialized()
{
    QJsObject* jsContext = getValidUIAbilityContext();
    if (jsContext == nullptr) {
        return;
    }

    QtOh::runOnJsUIThreadAndWait([this, jsContext] {
        initializeSystemConfiguration(jsContext);
        setupEnvironmentListener(jsContext);
        registerPcModeChangedCallback();
    });
}

bool QOhAppContextPrivate::darkColorModeActive() const
{
    return QGuiApplicationPrivate::platformTheme()->darkThemeEnabled();
}

QOhAppContext::QOhAppContext(QObject *parent) : QObject(*(new QOhAppContextPrivate()), parent)
{
    qRegisterMetaType<QOhAppContext::ColorMode>();
    qRegisterMetaType<QOhAppContext::SysThem>();
    d_func()->initialized();
}

QOhAppContext::~QOhAppContext() {}

QOhAppContext *QOhAppContext::instance()
{
    std::call_once(QOhAppContextPrivate::m_once,
        [] { QOhAppContextPrivate::m_appContext.reset(new QOhAppContext()); });

    return QOhAppContextPrivate::m_appContext.get();
}

QOhAppContext::SysThem QOhAppContext::sysThem() const
{
    Q_D(const QOhAppContext);
    return OhConfigurationColorMode::COLOR_MODE_DARK == d->m_sysColorMode ?\
            QOhAppContext::SysThem::Dark : QOhAppContext::SysThem::Light;
}

void QOhAppContext::setColorMode(ColorMode mode)
{
    Q_D(QOhAppContext);
    auto calcNewColorMode = [d](auto mode) {
        switch (mode) {
            case ColorMode::Dark:
                return ColorMode::Dark;
            case ColorMode::Light:
                return ColorMode::Light;
            case ColorMode::Default:
                return ColorMode::Default;
            case ColorMode::FollowSystemSetting:
                return ::sysColorMode2ColorMode(d->m_sysColorMode);
        };
        qWarning("%s: got unknown ColorThemeMode: %d", Q_FUNC_INFO, static_cast<int>(mode));
        return ColorMode::Default;
    };

    d->m_followSystemColorMode = (ColorMode::FollowSystemSetting == mode);
    auto newColorMode = calcNewColorMode(mode);
    if (d->m_colorMode != newColorMode) {
        d->m_colorMode = newColorMode;
        QGuiApplicationPrivate::platformTheme()->setThemeColorMode(int(newColorMode));
        Q_EMIT colorModeChanged(d->m_colorMode, QPrivateSignal());
    }
}

QOhAppContext::ColorMode QOhAppContext::colorMode() const
{
    Q_D(const QOhAppContext);
    return d->m_colorMode;
}

bool QOhAppContext::isPermissionGranted(const QString &permission) const
{
    return QtOh::checkPermission(permission) == QtOh::PermissionResult::Granted;
}

bool QOhAppContext::requestPermissionFromUser(const QString &permission) const
{
    auto result = QtOh::requestPermissionsSync({permission});
    if (result.authResults.isEmpty())
        return false;
    return result.authResults.first() == QtOh::PermissionResult::Granted;
}

bool QOhAppContext::requestPermissionsOnSetting(const QString &permission) const
{
    auto result = QtOh::requestPermissionsOnSettingSync({permission});
    if (result.authResults.isEmpty())
        return false;
    return result.authResults.first() == QtOh::PermissionResult::Granted;
}
