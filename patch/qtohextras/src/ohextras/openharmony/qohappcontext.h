/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * 文件名称: qohappcontext.h
 * 简要描述: 主题相关处理类
 * 创建日期: 2025/06/18
 * 作者: WangHao
 * 说明:
 *
 * 修改日期:
 * 作者:
 * 说明:
 * **************************************************************** */
#ifndef QOHAPPCONTEXT_H
#define QOHAPPCONTEXT_H
#include <QObject>
#include "qopenharmonyextrasglobal.h"

class QOhAppContextPrivate;
class Q_OPENHARMONYEXTRAS_EXPORT QOhAppContext : public QObject {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QOhAppContext)
    Q_DISABLE_COPY_MOVE(QOhAppContext)
    QOhAppContext(QObject *parent = nullptr);

public:
    enum class SysThem { Dark, Light };
    enum class ColorMode { Dark, Light, Default, FollowSystemSetting };
    ~QOhAppContext();
    static QOhAppContext *instance();

    ColorMode colorMode() const;
    QOhAppContext::SysThem sysThem() const;
    bool isPermissionGranted(const QString &permission) const;
    bool requestPermissionFromUser(const QString &permission) const;
    bool requestPermissionsOnSetting(const QString &permission) const;

public Q_SLOTS:
    void setColorMode(QOhAppContext::ColorMode mode);

Q_SIGNALS:
    void sysThemChanged(QOhAppContext::SysThem, QPrivateSignal);
    void colorModeChanged(QOhAppContext::ColorMode, QPrivateSignal);
    void pcModeChanged(bool isPCMode);
};
Q_DECLARE_METATYPE(QOhAppContext::SysThem)
Q_DECLARE_METATYPE(QOhAppContext::ColorMode)

#endif // QOHAPPCONTEXT_H
