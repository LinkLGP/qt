/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include <QDebug>
#include <QTimer>
#include <QWidget>
#include <QJsObject>
#include <qohutility.h>
#include <QGuiApplication>
#include <private/qobject_p.h>
#include <private/qjspromise_p.h>
#include <private/qopenharmony_p.h>
#include <private/qhighdpiscaling_p.h>
#include <qpa/qplatformnativeinterface.h>

#include "qohwidgethelper.h"

Q_LOGGING_CATEGORY(widgethelper, "ohos.widget.helper");

namespace {
constexpr int API_11 = 11;
constexpr int API_14 = 14;
constexpr int API_17 = 17;
constexpr int API_18 = 18;
}
QT_BEGIN_NAMESPACE
namespace {
bool isSubWindow(QWindow *w)
{
    if (!w) {
        return false;
    }

    /* 不处理嵌入式子窗口 */
    if (w->parent()) {
        return false;
    }

    /* 默认子窗 */
    Qt::WindowFlags flags = w->flags();
    if (flags.testFlag(Qt::ToolTip) || flags.testFlag(Qt::Popup)) {
        return true;
    }

    if (!w->transientParent()) {
        qWarning() << "interface only supports calling in independent child windows.";
        return false;
    }

    return true;
}
}
struct Data {
    explicit Data(QOhWidgetHelperPrivate *helper) : widgetHelper(helper) {}
    virtual ~Data();
    virtual bool set() = 0;
    virtual QVariant get() const = 0;
    QOhWidgetHelperPrivate *widgetHelper;
};

Data::~Data() = default;

class QOhPrivacyModeData final : public Data {
public:
    explicit QOhPrivacyModeData(QOhWidgetHelperPrivate *helper, bool isPrivacyMode = false)
        : Data(helper), m_isPrivacyMode(isPrivacyMode)
    {}
    ~QOhPrivacyModeData() override = default;
    bool set() override;
    virtual QVariant get() const override;

private:
    bool m_isPrivacyMode;
};

class QOhWindowRectAutoSaveData final : public Data {
public:
    explicit QOhWindowRectAutoSaveData(QOhWidgetHelperPrivate *helper, bool autoSave = false)
        : Data(helper), m_isAutoSave(autoSave)
    {}
    ~QOhWindowRectAutoSaveData() override = default;
    bool set() override;
    virtual QVariant get() const override;

private:
    bool m_isAutoSave;
};

static QOhWidgetHelper::DecorButtonStyle defaultDecorButtonStyle = { QOhWidgetHelper::ColorMode::COLOR_MODE_NOT_SET, 28,
    12, 20 };
class QOhDecorButtonStyleData final : public Data {
public:
    explicit QOhDecorButtonStyleData(QOhWidgetHelperPrivate *helper,
        const QOhWidgetHelper::DecorButtonStyle &style = defaultDecorButtonStyle)
        : Data(helper), m_style(style)
    {}
    ~QOhDecorButtonStyleData() override = default;
    bool set() override;
    virtual QVariant get() const override;

private:
    QOhWidgetHelper::DecorButtonStyle m_style;
};

class QOhWindowKeepScreenOnData final : public Data {
public:
    explicit QOhWindowKeepScreenOnData(QOhWidgetHelperPrivate *helper, bool keepOn = false)
        : Data(helper), m_keepOn(keepOn)
    {}
    ~QOhWindowKeepScreenOnData() override = default;
    bool set() override;
    virtual QVariant get() const override;

private:
    bool m_keepOn;
};

class QOhSupportedWindowModesData final : public Data {
public:
    explicit QOhSupportedWindowModesData(QOhWidgetHelperPrivate *helper,
        QOhWidgetHelper::SupportedWindowModes modes = QOhWidgetHelper::SupportedWindowModes())
        : Data(helper), m_modes(modes)
    {}
    ~QOhSupportedWindowModesData() override = default;
    bool set() override;
    virtual QVariant get() const override;

private:
    QOhWidgetHelper::SupportedWindowModes m_modes;
};

class QOhWindowBackgroundColorData final : public Data {
public:
    QOhWindowBackgroundColorData(QOhWidgetHelperPrivate *helper, const QColor &color) : Data(helper), m_color(color) {}
    ~QOhWindowBackgroundColorData() override = default;
    bool set() override;
    virtual QVariant get() const override;

private:
    QColor m_color;
};

class QOhWindowCornerData final : public Data {
public:
    explicit QOhWindowCornerData(QOhWidgetHelperPrivate *helper, qreal cornerRadius = 16)
        : Data(helper), m_cornerRadius(cornerRadius)
    {}
    ~QOhWindowCornerData() override = default;
    bool set() override;
    virtual QVariant get() const override;

private:
    qreal m_cornerRadius;
};

class QOhWindowShadowData final : public Data {
public:
    QOhWindowShadowData(QOhWidgetHelperPrivate *helper, qreal shadowRadius) : Data(helper), m_shadowRadius(shadowRadius)
    {}
    ~QOhWindowShadowData() override = default;
    bool set() override;
    virtual QVariant get() const override;

private:
    qreal m_shadowRadius;
};

class QOhWindowResizeByDragData final : public Data {
public:
    QOhWindowResizeByDragData(QOhWidgetHelperPrivate *helper, bool enable) : Data(helper), m_enable(enable) {}
    ~QOhWindowResizeByDragData() override = default;
    bool set() override;
    QVariant get() const override;

private:
    qreal m_enable;
};

class QOhWindowDecorData final : public Data {
public:
    explicit QOhWindowDecorData(QOhWidgetHelperPrivate *helper, bool visible = true) : Data(helper), m_visible(visible)
    {}
    ~QOhWindowDecorData() override = default;
    bool set() override;
    QVariant get() const override;

private:
    bool m_visible;
};

class QOhWidgetHelperPrivate : public QObjectPrivate {
    Q_DECLARE_PUBLIC(QOhWidgetHelper)
public:
    ~QOhWidgetHelperPrivate();
    QPointer<QWidget> m_widget;
    mutable QPointer<QWindow> m_window;
    QJsObject *native(const QByteArray &nativeName) const;
    QJsObject *jsWindow() const;
    QJsObject *windowStage() const;
    QWindow *window() const;
    QOhWidgetHelper::DecorButtonStyle formatStyle(const QOhWidgetHelper::DecorButtonStyle &style, bool toNative = true);
    QPlatformNativeInterface::NativeResourceForIntegrationFunction nativeFunctionForIntegration(
        const QByteArray &funcName);
    void registerHandler();
    bool registerPCModeChangedHandler();
    bool registerTitleButtonRectChangedHandler();
    void handleShowEvent(QOhWidgetHelper *helper);
    void handleHideEvent(QObject *watched);
    template <typename T, typename... Args> void set(Args &&... args)
    {
        QScopedPointer<T> data(new T(args...));
        if (!data->set()) {
            m_datas << data.take();
        }
    }
    template <typename T, typename RET> RET get() const
    {
        T data(const_cast<QOhWidgetHelperPrivate *>(this));
        return data.get().template value<RET>();
    }
    QList<Data *> m_datas;
    bool m_pcModeChangedHandlerRegistered = false;
    bool m_titleButtonRectChangedHandlerRegistered = false;
};


QOhWidgetHelper::QOhWidgetHelper(QWidget *widget, QObject *parent) : QObject(*new QOhWidgetHelperPrivate, parent)
{
    d_func()->m_widget = widget;
    if (widget != nullptr)
        widget->installEventFilter(this);
    d_func()->registerHandler();
}

QOhWidgetHelper::QOhWidgetHelper(QWindow *window, QObject *parent) : QObject(*new QOhWidgetHelperPrivate, parent)
{
    d_func()->m_window = window;
    if (window != nullptr)
        window->installEventFilter(this);
    d_func()->registerHandler();
}

QOhWidgetHelper::~QOhWidgetHelper()
{
    Q_D(QOhWidgetHelper);
    if (d->m_datas.isEmpty())
        return;
    qDeleteAll(d->m_datas);
    d->m_datas.clear();
}

void QOhWidgetHelper::setPrivacyMode(bool isPrivacyMode)
{
    Q_D(QOhWidgetHelper);

    d->set<QOhPrivacyModeData>(d, isPrivacyMode);
}

bool QOhWidgetHelper::isPrivacyMode() const
{
    Q_D(const QOhWidgetHelper);
    return d->get<QOhPrivacyModeData, bool>();
}

/* !
 * \brief 支持应用控制启动页消失时机
此接口只对应用主窗口生效，且需要在module.json5配置文件
abilities标签中的metadata标签下配置"enable.remove.starting.window"为"true"才会生效
在标签配置为"true"的情况下，系统提供了启动页超时保护机制，若5s内未调用此接口，系统将自动移除启动页
若标签配置为"false"或未配置标签，则此接口不生效，启动页将会在应用首帧渲染完成后自动移除。
 */
void QOhWidgetHelper::removeStartingWindow()
{
    Q_D(const QOhWidgetHelper);
    if (QtOh::apiVersion() < API_14) {
        qCWarning(widgethelper) << "This interface has been supported since API version 14.";
        return;
    }
    auto windowStage = d->windowStage();
    if (windowStage == nullptr)
        return;

    QtOh::runOnJsUIThreadNoWait([windowStage] { windowStage->call(QLatin1String("removeStartingWindow")); });
}

QRect QOhWidgetHelper::titleButtonRect() const
{
    Q_D(const QOhWidgetHelper);
    QRect rect = QtOh::runOnJsUIThreadWithResult([d] {
        auto window = d->jsWindow();
        if (window == nullptr)
            return QRect();
        Napi::Object result = window->call(QLatin1String("getTitleButtonRect")).As<Napi::Object>();
        return QRect(result.Get("left").ToNumber(), result.Get("top").ToNumber(), result.Get("width").ToNumber(),
            result.Get("height").ToNumber());
    });
    return QHighDpi::fromNativePixels(rect, d->window());
}

void QOhWidgetHelper::setDecorButtonStyle(const QOhWidgetHelper::DecorButtonStyle &style)
{
    Q_D(QOhWidgetHelper);

    d->set<QOhDecorButtonStyleData>(d, style);
}

void QOhWidgetHelper::setWindowKeepScreenOn(bool isKeepScreenOn)
{
    Q_D(QOhWidgetHelper);

    d->set<QOhWindowKeepScreenOnData>(d, isKeepScreenOn);
}

void QOhWidgetHelper::setSupportedWindowModes(SupportedWindowModes modes)
{
    Q_D(QOhWidgetHelper);

    d->set<QOhSupportedWindowModesData>(d, modes);
}

void QOhWidgetHelper::setWindowBackgroundColor(const QColor &color)
{
    Q_D(QOhWidgetHelper);

    d->set<QOhWindowBackgroundColorData>(d, color);
}

void QOhWidgetHelper::setWindowCornerRadius(qreal cornerRadius)
{
    if (QtOh::apiVersion() < API_17) {
        qCWarning(widgethelper) << "This interface has been supported since API version 17.";
        return;
    }
    Q_D(QOhWidgetHelper);

    d->set<QOhWindowCornerData>(d, cornerRadius);
}

qreal QOhWidgetHelper::windowCornerRadius()
{
    if (QtOh::apiVersion() < API_17) {
        qCWarning(widgethelper) << "This interface has been supported since API version 17.";
        return -1.f;
    }

    Q_D(QOhWidgetHelper);

    return d->get<QOhWindowCornerData, qreal>();
}

bool QOhWidgetHelper::windowDecorVisible() const
{
    Q_D(const QOhWidgetHelper);

    return d->get<QOhWindowDecorData, bool>();
}

void QOhWidgetHelper::setWindowDecorVisible(bool visible)
{
    Q_D(QOhWidgetHelper);

    d->set<QOhWindowDecorData>(d, visible);
}

void QOhWidgetHelper::setResizeByDragEnabled(bool enable)
{
    Q_D(QOhWidgetHelper);

    d->set<QOhWindowResizeByDragData>(d, enable);
}

void QOhWidgetHelper::setWindowShadowRadius(qreal shadowRadius)
{
    if (QtOh::apiVersion() < API_17) {
        qCWarning(widgethelper) << "This interface has been supported since API version 17.";
        return;
    }

    Q_D(QOhWidgetHelper);
    d->set<QOhWindowShadowData>(d, shadowRadius);
}

void QOhWidgetHelper::setWindowTitleMoveEnabled(bool enable)
{
    if (QtOh::apiVersion() < API_14) {
        qCWarning(widgethelper) << "This interface has been supported since API version 14.";
        return;
    }

    Q_D(QOhWidgetHelper);
    auto isFreeWindowEnable = d->nativeFunctionForIntegration("isFreeWindowEnable");
    if (QtOh::Utility::type() == QtOh::Utility::Tablet && !*reinterpret_cast<bool *>(isFreeWindowEnable())) {
        qCWarning(widgethelper, "setWindowTitleMoveEnabled This interface is only available in Free Multi-Window Mode "
                                "on 2-in-1 devices or tablet devices.");
        return;
    }

    auto jsObject = d->jsWindow();
    if (nullptr == jsObject) {
        qCWarning(widgethelper, "get native window failed.");
        return;
    }

    QtOh::runOnJsUIThreadNoWait([jsObject, enable] {
        jsObject->call("setWindowTitleMoveEnabled", { Napi::Boolean::New(jsObject->env(), enable) });
    });
}

void QOhWidgetHelper::setWindowTitleButtonVisible(bool maximizeButtonVisible, bool minimizeButtonVisible,
    bool closeButtonVisible)
{
    Q_D(QOhWidgetHelper);
    auto isFreeWindowEnable = d->nativeFunctionForIntegration("isFreeWindowEnable");
    if (QtOh::Utility::type() == QtOh::Utility::Tablet && !*reinterpret_cast<bool *>(isFreeWindowEnable())) {
        qCWarning(widgethelper, "setWindowTitleButtonVisible This interface is only available in Free Multi-Window "
                                "Mode on 2-in-1 devices or tablet devices.");
        return;
    }

    auto jsObject = d->jsWindow();
    if (nullptr == jsObject) {
        qCWarning(widgethelper, "get native window failed.");
        return;
    }

    QtOh::runOnJsUIThreadNoWait([jsObject, maximizeButtonVisible, minimizeButtonVisible, closeButtonVisible] {
        if (QtOh::apiVersion() < API_14) {
            qCWarning(widgethelper, "API versions below 14 do not support hiding the close button.");
            jsObject->call("setWindowTitleButtonVisible", { Napi::Boolean::New(jsObject->env(), maximizeButtonVisible),
                Napi::Boolean::New(jsObject->env(), minimizeButtonVisible) });
        } else {
            jsObject->call("setWindowTitleButtonVisible", { Napi::Boolean::New(jsObject->env(), maximizeButtonVisible),
                Napi::Boolean::New(jsObject->env(), minimizeButtonVisible),
                Napi::Boolean::New(jsObject->env(), closeButtonVisible) });
        }
    });
}

QOhWidgetHelper::DecorButtonStyle QOhWidgetHelper::decorButtonStyle() const
{
    Q_D(const QOhWidgetHelper);
    return d->get<QOhDecorButtonStyleData, DecorButtonStyle>();
}

bool QOhWidgetHelper::eventFilter(QObject *watched, QEvent *event)
{
    // 鸿蒙系统上，只有当窗口显示之后才会创建NativeWindow, Show事件发送时，本地窗口还没有创建
    // 发送到队列中执行
    Q_D(QOhWidgetHelper);
    
    if (watched != d->m_widget.data() && watched != d->m_window.data()) {
        return QObject::eventFilter(watched, event);
    }
    
    if (event->type() == QEvent::Show) {
        d->handleShowEvent(this);
    } else if (event->type() == QEvent::Hide) {
        d->handleHideEvent(watched);
    }
    
    return QObject::eventFilter(watched, event);
}

void QOhWidgetHelper::setWindowRectAutoSave(bool enable)
{
    if (QtOh::apiVersion() < API_14) {
        qCWarning(widgethelper) << "This interface has been supported since API version 14.";
        return;
    }
    Q_D(QOhWidgetHelper);
    d->set<QOhWindowRectAutoSaveData>(d, enable);
}

bool QOhWidgetHelper::isWindowRectAutoSave() const
{
    if (QtOh::apiVersion() < API_14) {
        qCWarning(widgethelper) << "This interface has been supported since API version 14.";
        return false;
    }

    Q_D(const QOhWidgetHelper);
    return d->get<QOhWindowRectAutoSaveData, bool>();
}

QOhWidgetHelperPrivate::~QOhWidgetHelperPrivate() {}

QJsObject *QOhWidgetHelperPrivate::native(const QByteArray &nativeName) const
{
    if (window() == nullptr) {
        return nullptr;
    }
    auto platformNativeInterface = qApp->platformNativeInterface();
    if (platformNativeInterface == nullptr) {
        return nullptr;
    }
    QJsObject *obj =
        reinterpret_cast<QJsObject *>(platformNativeInterface->nativeResourceForWindow(nativeName, window()));
    return obj;
}

QJsObject *QOhWidgetHelperPrivate::jsWindow() const
{
    return native("nativeWindow");
}

QJsObject *QOhWidgetHelperPrivate::windowStage() const
{
    return native("nativeWindowStage");
}

QWindow *QOhWidgetHelperPrivate::window() const
{
    if (!m_window.isNull()) {
        return m_window.data();
    }
#if defined(ENABLE_HERE)
    QWidget *w = m_widget->nativeParentWidget();
    if (w == nullptr) {
        w = m_widget.data();
    }
#endif
    if (!m_widget) {
        return nullptr;
    }

    auto windowHandle = m_widget->windowHandle();
    m_window = windowHandle;
    return windowHandle;
}

QOhWidgetHelper::DecorButtonStyle QOhWidgetHelperPrivate::formatStyle(const QOhWidgetHelper::DecorButtonStyle &style,
    bool toNative)
{
    auto windowHandle = window();
    if (toNative) {
        return { style.colorMode, QHighDpi::toNativePixels(style.buttonBackgroundSize, windowHandle),
            QHighDpi::toNativePixels(style.spacingBetweenButtons, windowHandle),
            QHighDpi::toNativePixels(style.closeButtonRightMargin, windowHandle) };
    }
    return { style.colorMode, QHighDpi::fromNativePixels(style.buttonBackgroundSize, windowHandle),
        QHighDpi::fromNativePixels(style.spacingBetweenButtons, windowHandle),
        QHighDpi::fromNativePixels(style.closeButtonRightMargin, windowHandle) };
}

QPlatformNativeInterface::NativeResourceForIntegrationFunction QOhWidgetHelperPrivate::nativeFunctionForIntegration(
    const QByteArray &funcName)
{
    auto platformNativeInterface = qApp->platformNativeInterface();
    if (platformNativeInterface == nullptr) {
        return nullptr;
    }

    return platformNativeInterface->nativeResourceFunctionForIntegration(funcName);
}

void QOhWidgetHelperPrivate::registerHandler()
{
    if (!m_pcModeChangedHandlerRegistered) {
        m_pcModeChangedHandlerRegistered = registerPCModeChangedHandler();
    }
    if (!m_titleButtonRectChangedHandlerRegistered) {
        m_titleButtonRectChangedHandlerRegistered = registerTitleButtonRectChangedHandler();
    }
}

void QOhWidgetHelperPrivate::handleShowEvent(QOhWidgetHelper *helper)
{
    if (!m_datas.isEmpty()) {
        QTimer::singleShot(0, helper, [this] {
            for (int i = 0; i < m_datas.count(); ++i) {
                m_datas.at(i)->set();
            }
            qDeleteAll(m_datas);
            m_datas.clear();
        });
    }
    QTimer::singleShot(0, helper, [this] {
        registerHandler();
    });
}

void QOhWidgetHelperPrivate::handleHideEvent(QObject *watched)
{
    if (watched->parent() != nullptr) {
        return;
    }
    m_pcModeChangedHandlerRegistered = false;
    m_titleButtonRectChangedHandlerRegistered = false;
}

bool QOhWidgetHelperPrivate::registerPCModeChangedHandler()
{
    QJsObject *window = jsWindow();
    if (window == nullptr)
        return false;
    Q_Q(QOhWidgetHelper);
    QtOh::runOnJsUIThreadAndWait([guard = QPointer<QOhWidgetHelper>(q), window] {
        Napi::Function pcModeChangedListener = Napi::Function::New(window->env(),
        [guard](const Napi::CallbackInfo& info) {
            if (guard.isNull() || info.Length() < 1 || !info[0].IsBoolean())
                return;
            bool pcMode = info[0].ToBoolean();
            emit guard->pcModeChanged(pcMode);
        });
        window->object().Set("pcModeChangedListener", pcModeChangedListener);
    });
    return true;
}

bool QOhWidgetHelperPrivate::registerTitleButtonRectChangedHandler()
{
    auto window = jsWindow();
    if (window == nullptr)
        return false;

    Q_Q(QOhWidgetHelper);
    QtOh::runOnJsUIThreadAndWait([guard = QPointer<QOhWidgetHelper>(q), window] {
        window->call(QLatin1String("on"), {Napi::String::New(window->env(), "windowTitleButtonRectChange"),
            Napi::Function::New(window->env(), [guard](const Napi::CallbackInfo& info) {
                                                Napi::Object titleButtonRect = info[0].As<Napi::Object>();
                                                QRect rect(titleButtonRect.Get("left").ToNumber(),
                                                           titleButtonRect.Get("top").ToNumber(),
                                                           titleButtonRect.Get("width").ToNumber(),
                                                           titleButtonRect.Get("height").ToNumber());
                                                if (!guard.isNull())
                                                    emit guard->titleButtonRectChanged(rect);
                                            })});
    });
    return true;
}

bool QOhPrivacyModeData::set()
{
    QJsObject *window = widgetHelper->jsWindow();
    if (window == nullptr) {
        return false;
    }
    QtOh::runOnJsUIThreadAndWait([this, window] {
        window->call(QString::fromUtf8("setWindowPrivacyMode"), { Napi::Boolean::New(window->env(), m_isPrivacyMode) });
    });
    return true;
}

QVariant QOhPrivacyModeData::get() const
{
    QJsObject *window = widgetHelper->jsWindow();
    if (window == nullptr) {
        return false;
    }
    return QtOh::runOnJsUIThreadWithResult([window] {
        Napi::Object result = window->call(QString::fromUtf8("getWindowProperties")).As<Napi::Object>();
        bool isPrivacy = result.Get("isPrivacyMode").ToBoolean();
        return isPrivacy;
    });
}

bool QOhWindowRectAutoSaveData::set()
{
    QJsObject *ws = widgetHelper->windowStage();
    if (ws == nullptr) {
        return false;
    }
    QtOh::runOnJsUIThreadAndWait([this, ws] {
        ws->call(QString::fromUtf8("setWindowRectAutoSave"), { Napi::Boolean::New(ws->env(), m_isAutoSave) });
    });
    return true;
}

QVariant QOhWindowRectAutoSaveData::get() const
{
    QJsObject *ws = widgetHelper->windowStage();
    if (ws == nullptr) {
        return false;
    }

    return QtOh::runOnJsUIThreadWithPromise<bool>([ws](auto p) {
        QJsPromise promise(ws->call(QString::fromUtf8("isWindowRectAutoSave")).As<Napi::Promise>());
        promise
            .onThen([p](const Napi::CallbackInfo &info) {
                bool isAutoSave = info[0].ToBoolean();
                p->set_value(isAutoSave);
            })
            .onCatch([p](const Napi::CallbackInfo &info) {
                Q_UNUSED(info)
                p->set_value(false);
            });
    });
}

bool QOhDecorButtonStyleData::set()
{
    QJsObject *window = widgetHelper->jsWindow();
    if (window == nullptr) {
        return false;
    }
    QtOh::runOnJsUIThreadAndWait([style = widgetHelper->formatStyle(m_style), window] {
        Napi::Object jsStyle = Napi::Object::New(window->env());
        jsStyle.Set("colorMode", (int)style.colorMode);
        jsStyle.Set("buttonBackgroundSize", style.buttonBackgroundSize);
        jsStyle.Set("spacingBetweenButtons", style.spacingBetweenButtons);
        jsStyle.Set("closeButtonRightMargin", style.closeButtonRightMargin);
        window->call(QString::fromUtf8("setDecorButtonStyle"), { jsStyle });
    });
    return true;
}

QVariant QOhDecorButtonStyleData::get() const
{
    QVariant v;
    v.setValue(widgetHelper->formatStyle(defaultDecorButtonStyle, false));

    QJsObject *window = widgetHelper->jsWindow();
    if (window == nullptr) {
        return v;
    }
    QOhWidgetHelper::DecorButtonStyle style = QtOh::runOnJsUIThreadWithResult([window] {
        Napi::Object result = window->call(QString::fromUtf8("getDecorButtonStyle")).As<Napi::Object>();
        return QOhWidgetHelper::DecorButtonStyle{ (QOhWidgetHelper::ColorMode)((int)result.Get("colorMode").ToNumber()),
            result.Get("buttonBackgroundSize").ToNumber(), result.Get("spacingBetweenButtons").ToNumber(),
            result.Get("closeButtonRightMargin").ToNumber() };
    });
    v.setValue(widgetHelper->formatStyle(style, false));
    return v;
}

bool QOhWindowKeepScreenOnData::set()
{
    QJsObject *window = widgetHelper->jsWindow();
    if (window == nullptr) {
        return false;
    }
    QtOh::runOnJsUIThreadNoWait([keepOn = m_keepOn, window] {
        window->call(QLatin1String("setWindowKeepScreenOn"), { Napi::Boolean::New(window->env(), keepOn) });
    });
    return true;
}

QVariant QOhWindowKeepScreenOnData::get() const
{
    QJsObject *window = widgetHelper->jsWindow();
    if (window == nullptr)
        return false;
    return QtOh::runOnJsUIThreadWithResult([window] {
        Napi::Object result = window->call(QString::fromUtf8("getWindowProperties")).As<Napi::Object>();
        bool isKeepScreenOn = result.Get("isKeepScreenOn").ToBoolean();
        return isKeepScreenOn;
    });
}

namespace {
constexpr int FULL_SCREEN_VALUE = 0; /* 窗口支持全屏显示 */
constexpr int SPLIT_VALUE = 1;       /* 窗口支持分屏显示 */
constexpr int FLOATING_VALUE = 2;    /* 支持窗口化显示 */
}

bool QOhSupportedWindowModesData::set()
{
    QJsObject *ws = widgetHelper->windowStage();
    if (ws == nullptr)
        return false;
    if (m_modes == QOhWidgetHelper::SupportedWindowModes())
        return true;
    QtOh::runOnJsUIThreadNoWait([modes = m_modes, ws] {
        Napi::Array arr = Napi::Array::New(ws->env());
        int index = 0;
        if (modes.testFlag(QOhWidgetHelper::SPLIT))
            arr.Set((uint32_t)index++, Napi::Number::New(ws->env(), SPLIT_VALUE));
        if (modes.testFlag(QOhWidgetHelper::FLOATING))
            arr.Set((uint32_t)index++, Napi::Number::New(ws->env(), FLOATING_VALUE));
        if (modes.testFlag(QOhWidgetHelper::FULL_SCREEN))
            arr.Set((uint32_t)index++, Napi::Number::New(ws->env(), FULL_SCREEN_VALUE));
        ws->call(QLatin1String("setSupportedWindowModes"), { arr });
    });
    return true;
}

QVariant QOhSupportedWindowModesData::get() const
{
    return (int)QOhWidgetHelper::SPLIT | QOhWidgetHelper::FLOATING | QOhWidgetHelper::FULL_SCREEN;
}


bool QOhWindowBackgroundColorData::set()
{
    QJsObject *jw = widgetHelper->jsWindow();
    if (jw == nullptr)
        return false;

    QString colorStr = m_color.name(QColor::HexArgb);
    QtOh::runOnJsUIThreadNoWait([colorStr, jw] {
        jw->call("setWindowBackgroundColor", { Napi::String::New(jw->env(), colorStr.toStdString()) });
    });
    
    return true;
}

QVariant QOhWindowBackgroundColorData::get() const
{
    return QVariant();
}

bool QOhWindowCornerData::set()
{
    QJsObject *jw = widgetHelper->jsWindow();
    if (!jw) {
        return false;
    }

    if (!isSubWindow(widgetHelper->window())) {
        return false;
    }

    QtOh::runOnJsUIThreadNoWait([jw, cornerRadius = m_cornerRadius] {
        jw->call("setWindowCornerRadius", { Napi::Number::New(jw->env(), cornerRadius) });
    });
    return true;
}

QVariant QOhWindowCornerData::get() const
{
    qreal invalidValue = -1;

    QJsObject *jw = widgetHelper->jsWindow();
    if (!jw) {
        return invalidValue;
    }

    if (!isSubWindow(widgetHelper->window())) {
        return invalidValue;
    }

    return QtOh::runOnJsUIThreadWithResult([jw, invalidValue] {
        Napi::Value ret = jw->call("getWindowCornerRadius");
        if (ret.IsNumber()) {
            qreal result = ret.ToNumber();
            return result;
        }

        return invalidValue;
    });
}

bool QOhWindowShadowData::set()
{
    QJsObject *jw = widgetHelper->jsWindow();
    if (!jw) {
        return false;
    }

    if (!isSubWindow(widgetHelper->window())) {
        return false;
    }

    QtOh::runOnJsUIThreadNoWait([jw, shadowRadius = m_shadowRadius] {
        jw->call("setWindowShadowRadius", { Napi::Number::New(jw->env(), shadowRadius) });
    });
    return true;
}

QVariant QOhWindowShadowData::get() const
{
    return QVariant();
}

bool QOhWindowResizeByDragData::set()
{
    if (QtOh::apiVersion() < API_14) {
        qCWarning(widgethelper) << "This interface has been supported since API version 14.";
        return false;
    }

    QJsObject *jw = widgetHelper->jsWindow();
    if (jw == nullptr)
        return false;

    QtOh::runOnJsUIThreadNoWait([jw, this] {
        Napi::Value ret = jw->call("setResizeByDragEnabled", { Napi::Number::New(jw->env(), m_enable) });
        Napi::Promise p = ret.As<Napi::Promise>();
        if (p.IsNull()) {
            return;
        }
        QJsPromise promise(p);
        promise.onThen([](const Napi::CallbackInfo &info) { ; }).onCatch([](const Napi::CallbackInfo &info) {
            Napi::Object error = info[0].As<Napi::Object>();
            int code = (int)error.Get("code").ToNumber();
            if (code != 0) {
                LOGW("Failed to set the function of disabling the resize by drag window."
                    "code is %{public}d, message is %{public}s",
                    code, error.Get("message").ToString().Utf8Value().c_str());
            }
        });
    });

    return true;
}

QVariant QOhWindowResizeByDragData::get() const
{
    return QVariant();
}

bool QOhWindowDecorData::set()
{
    if (QtOh::apiVersion() < API_11) {
        qCWarning(widgethelper) << "This interface has been supported since API version 11.";
        return false;
    }

    QJsObject *jw = widgetHelper->jsWindow();
    if (jw == nullptr)
        return false;

    QtOh::runOnJsUIThreadNoWait(
        [jw, this] { jw->call("setWindowDecorVisible", { Napi::Number::New(jw->env(), m_visible) }); });

    return true;
}

QVariant QOhWindowDecorData::get() const
{
    if (QtOh::apiVersion() < API_18) {
        qCWarning(widgethelper) << "This interface has been supported since API version 18.";
        return false;
    }

    auto isFreeWindowEnable = widgetHelper->nativeFunctionForIntegration("isFreeWindowEnable");
    if (QtOh::Utility::type() == QtOh::Utility::Tablet && !*reinterpret_cast<bool *>(isFreeWindowEnable())) {
        qCWarning(widgethelper, "getWindowDecorVisible This interface is only available in Free Multi-Window Mode on "
                                "2-in-1 devices or tablet devices.");
        return false;
    }

    QJsObject *jw = widgetHelper->jsWindow();
    if (jw == nullptr)
        return false;

    return QtOh::runOnJsUIThreadWithResult([jw] {
        Napi::Value ret = jw->call("getWindowDecorVisible");
        if (ret.IsNumber()) {
            bool result = ret.ToBoolean();
            return result;
        }

        return false;
    });
}

QT_END_NAMESPACE
