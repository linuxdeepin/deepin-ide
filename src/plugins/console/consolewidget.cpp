// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "consolewidget.h"

#include <DMenu>

#include <QDebug>

DWIDGET_USE_NAMESPACE

static ConsoleWidget *ins{nullptr};

class ConsoleWidgetPrivate
{
public:
    DMenu *menu = nullptr;
    QAction *consoleCopy = nullptr;
    QAction *consolePaste = nullptr;
};

ConsoleWidget *ConsoleWidget::instance()
{
    if (!ins)
        ins = new ConsoleWidget;
    return ins;
}

ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QTermWidget(parent),
     d(new ConsoleWidgetPrivate())
{
    setMargin(0);
    setTerminalOpacity(0);
    setForegroundRole(QPalette::ColorRole::Window);
    setAutoFillBackground(true);
    setTerminalOpacity(1);

    auto theme = DGuiApplicationHelper::instance()->themeType();
    updateColorScheme(theme);
    if (availableKeyBindings().contains("linux"))
        setKeyBindings("linux");

    setScrollBarPosition(QTermWidget::ScrollBarRight);
    setTerminalSizeHint(false);
    setAutoClose(false);
    changeDir(QDir::homePath());
    sendText("clear\n");

    d->consoleCopy = new QAction(tr("copy"), this);
    d->consolePaste = new QAction(tr("paste"), this);
    QObject::connect(d->consoleCopy, &QAction::triggered, this, &QTermWidget::copyClipboard);
    QObject::connect(d->consolePaste, &QAction::triggered, this, &QTermWidget::pasteClipboard);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &ConsoleWidget::updateColorScheme);
}

ConsoleWidget::~ConsoleWidget()
{
    delete d;
}

void ConsoleWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (nullptr == d->menu) {
        d->menu = new DMenu(this);
        d->menu->setParent(this);
        d->menu->addAction(d->consoleCopy);
        d->menu->addAction(d->consolePaste);
    }
    if (selectedText().isEmpty()) {
        d->consoleCopy->setEnabled(false);
    } else {
        d->consoleCopy->setEnabled(true);
    }
    d->menu->exec(event->globalPos());
}

void ConsoleWidget::updateColorScheme(DGuiApplicationHelper::ColorType themetype)
{
    if (themetype == DGuiApplicationHelper::DarkType
        && availableColorSchemes().contains("Linux"))
        this->setColorScheme("Linux");
    else if (availableColorSchemes().contains("BlackOnWhite"))
        this->setColorScheme("BlackOnWhite");
}
