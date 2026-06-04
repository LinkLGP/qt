/* ***************************************************************************
 *
 * Copyright (C) 2025 iSoftStone. All rights reserved.
 * See LGPL for detailed Information
 *
 * This file is part of the qtohextras module.
 *
 * ************************************************************************** */
#include "mainwindow.h"

#include <QIntValidator>
#include <QComboBox>
#include <QOpenHarmonyWant>
#include <qopenharmonyability.h>
#include <QDebug>

#include "ui_mainwindow.h"

namespace {
/* 列索引常量 */
constexpr int COLUMN_KEY = 0;
constexpr int COLUMN_VALUE = 1;
constexpr int COLUMN_TYPE = 2;

/* 设置flags输入范围：最小值为0，最大值为100 */
constexpr int MIN_FLAGS_VALUE = 0;
constexpr int MAX_FLAGS_VALUE = 100;
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QIntValidator *v = new QIntValidator(this);
    v->setRange(MIN_FLAGS_VALUE, MAX_FLAGS_VALUE);
    ui->lineEdit_flags->setValidator(v);
    connect(ui->pushButton_add, &QPushButton::clicked, this, &MainWindow::onPushButtonAddClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &MainWindow::onPushButtonDeleteClicked);
    connect(ui->pushButton_ok, &QPushButton::clicked, this, &MainWindow::onPushButtonOkClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onPushButtonAddClicked()
{
    QTableWidgetItem *item = new QTableWidgetItem("key");
    ui->tableWidget->insertRow(0);
    ui->tableWidget->setItem(0, COLUMN_KEY, item);
    item = new QTableWidgetItem("value");
    ui->tableWidget->setItem(0, COLUMN_VALUE, item);
    QComboBox *box = new QComboBox;
    box->addItems(QStringList() << "string"
                                << "int"
                                << "float"
                                << "bool");
    ui->tableWidget->setCellWidget(0, COLUMN_TYPE, box);
}

void MainWindow::onPushButtonDeleteClicked()
{
    int index = ui->tableWidget->currentRow();
    ui->tableWidget->removeRow(index);
}

void MainWindow::onPushButtonOkClicked()
{
    QOpenHarmonyWant want;
    want.deviceId = ui->lineEdit_deviceId->text();
    want.bundleName = ui->lineEdit_bundleName->text();
    want.abilityName = ui->lineEdit_abilityName->text();
    want.moduleName = ui->lineEdit_moduleName->text();
    want.action = ui->lineEdit_action->text();
    want.entities = ui->lineEdit_entities->text().split(";");
    want.uri = ui->lineEdit_uri->text();
    want.type = ui->lineEdit_type->text();
    want.flags = ui->lineEdit_flags->text().toInt();
    QVariantMap parameters;
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem *item = ui->tableWidget->item(i, COLUMN_KEY);
        QString key = item->text();
        item = ui->tableWidget->item(i, COLUMN_VALUE);
        QString value = item->text();
        item = ui->tableWidget->item(i, COLUMN_TYPE);
        QString type = item->text();
        QVariant v;
        if (type == "int") {
            v = value.toInt();
        } else if (type == "bool") {
            v = (value == "true") || (value == "1");
        } else if (type == "float") {
            v = value.toFloat();
        } else {
            v = value;
        }
        want.parameters.insert(key, v);
    }
    bool result = QOpenHarmonyAbility::start(want);
    if (!result) {
        qWarning() << "open app failed";
    }
}
