// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ABSTRACTEDITWIDGET_H
#define ABSTRACTEDITWIDGET_H

#include <QWidget>

class AbstractEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractEditWidget(QWidget *parent = nullptr);

    virtual void closeWidget() {}
};

#endif   // ABSTRACTEDITWIDGET_H
