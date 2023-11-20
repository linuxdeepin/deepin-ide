// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractnav.h"
#include "abstractmenubar.h"
#include "abstractmainwindow.h"

#include <DMainWindow>
#include <DToolBar>
#include <DMenuBar>

#include <QDebug>
#include <QObject>

DWIDGET_USE_NAMESPACE

class AbstractMainWindowPrivate
{
    friend class AbstractMainWindow;
    DMainWindow* mainwindow = nullptr;
};

AbstractMainWindow::~AbstractMainWindow()
{
    if (d) {
        delete d;
    }
}

void *AbstractMainWindow::qMainWindow()
{
    return static_cast<void*>(d->mainwindow);
}

AbstractMainWindow::AbstractMainWindow(void *qMainWindow)
    : d(new AbstractMainWindowPrivate())
{
    if (!qMainWindow) {
        qCritical() << "Failed, use QMainWindow(0x0) to AbstractMainWindow";
        abort();
    }

    d->mainwindow = static_cast<DMainWindow*>(qMainWindow);
    DMainWindow::connect(d->mainwindow, &DMainWindow::destroyed,
                         d->mainwindow ,[this]() {
        delete this;
    }, Qt::UniqueConnection);
}
