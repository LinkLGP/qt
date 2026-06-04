/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include "qopenharmonyability.h"
#include <qohosabilityctrl.h>
#include "qopenharmonydefines.h"

#include <qdebug.h>
#include <qjsobject.h>
#include <private/qopenharmony_p.h>
#include <qpa/qplatformnativeinterface.h>
#include <QtGui/private/qguiapplication_p.h>

namespace QOpenHarmonyAbility {
static QVariantMap wantToMap(const QOpenHarmonyWant &want)
{
    QVariantMap map;
    if (!want.deviceId.isEmpty())
        map.insert("deviceId", want.deviceId);

    if (!want.bundleName.isEmpty())
        map.insert("bundleName", want.bundleName);

    if (!want.moduleName.isEmpty())
        map.insert("moduleName", want.moduleName);

    if (!want.abilityName.isEmpty())
        map.insert("abilityName", want.abilityName);

    if (!want.action.isEmpty())
        map.insert("action", want.action);

    if (!want.entities.isEmpty())
        map.insert("action", want.entities);

    if (!want.uri.isEmpty())
        map.insert("uri", want.uri);

    if (!want.type.isEmpty())
        map.insert("type", want.type);

    if (!want.parameters.isEmpty())
        map.insert("parameters", want.parameters);

    map.insert("flags", want.flags);
    return map;
}

static QVariantMap abilitResultToMap(const QOpenHarmonyAbilityResult &abilityResult)
{
    QVariantMap map = wantToMap(abilityResult.want);
    map.insert("resultCode", abilityResult.resultCode);
    return map;
}

static QVariantMap startOptionsToMap(const QOpenHarmonyStartOptions &startOptions)
{
    QVariantMap map;
    if (startOptions.windowMode != DEFAULT_VAL)
        map.insert("windowMode", startOptions.windowMode);

    map.insert("displayId", startOptions.displayId);
    map.insert("withAnimation", startOptions.withAnimation);

    if (startOptions.windowLeft != DEFAULT_VAL)
        map.insert("windowLeft", startOptions.windowLeft);
    if (startOptions.windowTop != DEFAULT_VAL)
        map.insert("windowTop", startOptions.windowTop);
    if (startOptions.windowWidth != DEFAULT_VAL)
        map.insert("windowWidth", startOptions.windowWidth);
    if (startOptions.windowHeight != DEFAULT_VAL)
        map.insert("windowHeight", startOptions.windowHeight);
    if (startOptions.processMode != DEFAULT_VAL)
        map.insert("processMode", startOptions.processMode);
    if (startOptions.startupVisibility != DEFAULT_VAL)
        map.insert("startupVisibility", startOptions.startupVisibility);
    if (!startOptions.startWindowIcon.isNull())
        map.insert("startWindowIcon", startOptions.startWindowIcon);
    if (!startOptions.startWindowBackgroundColor.isEmpty())
        map.insert("startWindowBackgroundColor", startOptions.startWindowBackgroundColor);
    if (!startOptions.supportWindowModes.isEmpty()) {
        QVariantList varList;
        for (const auto &mode : startOptions.supportWindowModes) {
            if (mode >= SupportWindowMode::FULL_SCREEN && mode <= SupportWindowMode::FLOATING) {
                varList.append(static_cast<int>(mode));
            }
        }
        map.insert("supportWindowModes", varList);
    }

    return map;
}


void start(const QOpenHarmonyWant &want, const QOpenHarmonyStartOptions &startOptions, QAbilityResultReceiver *receiver)
{
    auto handlerResult = [receiver](const Napi::Value &err) {
        if (receiver != nullptr)
            receiver->handleResult(err);
    };
    QtOhPrivate::startAbility(wantToMap(want), startOptionsToMap(startOptions), handlerResult);
}

void start(const QOpenHarmonyWant &want)
{
    return start(want, QOpenHarmonyStartOptions(), nullptr);
}

void start(const QOpenHarmonyWant &want, QAbilityResultReceiver *receiver)
{
    return start(want, QOpenHarmonyStartOptions(), receiver);
}

void startForResult(const QOpenHarmonyWant &want, const QOpenHarmonyStartOptions &startOptions,
    QAbilityResultReceiver *receiver)
{
    auto handlerResult = [receiver](const Napi::Value &err, const Napi::Value &result = Napi::Value()) {
        if (receiver != nullptr)
            receiver->handleResult(err, result);
    };
    QtOhPrivate::startAbilityForResult(wantToMap(want), startOptionsToMap(startOptions), handlerResult);
}

void startForResult(const QOpenHarmonyWant &want)
{
    return startForResult(want, QOpenHarmonyStartOptions(), nullptr);
}

void startForResult(const QOpenHarmonyWant &want, QAbilityResultReceiver *receiver)
{
    return startForResult(want, QOpenHarmonyStartOptions(), receiver);
}

void terminateSelfWithResult(const QOpenHarmonyAbilityResult &abilityResult)
{
    QtOhPrivate::terminateSelfWithResult(abilitResultToMap(abilityResult));
}

QJsObject *getEntryUIAbility()
{
    void *uiAbility =
            QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("UIAbility");
    if (!uiAbility) {
        qWarning() << "UIAbility is null.";
        return nullptr;
    }

    QJsObject *jsUiAbility = std::bit_cast<QJsObject *>(uiAbility);
    if (!jsUiAbility) {
        qWarning() << Q_FUNC_INFO << "get uiability failed.";
    }

    return jsUiAbility;
}

int getSubProcessPid(const QString &processIdent)
{
    QJsObject *jsUiAbility = getEntryUIAbility();
    if (!jsUiAbility) {
        qWarning() << Q_FUNC_INFO << "getSubProcessPid get uiability failed.";
        return -1;
    }

    if (jsUiAbility->object().Has("getSubProcessPid")) {
        return QtOh::runOnJsUIThreadWithResult([processIdent, jsUiAbility] {
            return jsUiAbility->call("getSubProcessPid", {Napi::String::New(QtOh::uiEnv(), processIdent.toStdString())})
                    .As<Napi::Number>().Int32Value();
        });
    } else {
        qWarning() << "UIEntryAbility have not getSubProcessPid func";
        return -1;
    }
}

bool getSubProcessAliveState(const QString &processIdent)
{
    QJsObject *jsUiAbility = getEntryUIAbility();
    if (!jsUiAbility) {
        qWarning() << Q_FUNC_INFO << "getSubProcessAliveState get uiability failed.";
        return false;
    }

    if (jsUiAbility->object().Has("getSubProcessAliveState")) {
        return QtOh::runOnJsUIThreadWithResult([processIdent, jsUiAbility] {
            return jsUiAbility
                    ->call("getSubProcessAliveState",
                           { Napi::String::New(QtOh::uiEnv(), processIdent.toStdString()) })
                    .As<Napi::Boolean>();
        });
    } else {
        qWarning() << "UIEntryAbility have not getSubProcessAliveState func";
        return false;
    }
}

void killSubProcess(const QString &processIdent)
{
    QJsObject *jsUiAbility = getEntryUIAbility();
    if (!jsUiAbility) {
        qWarning() << Q_FUNC_INFO << "killSubProcess get uiability failed.";
        return;
    }

    if (jsUiAbility->object().Has("killSubProcess")) {
        QtOh::runOnJsUIThreadNoWait([processIdent, jsUiAbility] {
            jsUiAbility->call("killSubProcess",
                              { Napi::String::New(QtOh::uiEnv(), processIdent.toStdString()) });
        });
    } else {
        qWarning() << "UIEntryAbility have not killSubProcess func";
    }
}

QAbilityResultReceiver::QAbilityResultReceiver() {}

QAbilityResultReceiver::~QAbilityResultReceiver() {}
}
