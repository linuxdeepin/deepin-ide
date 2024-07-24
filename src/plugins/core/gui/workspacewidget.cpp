// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"

#include <DTitlebar>
#include <DTabBar>
#include <DStyle>
#include <DStackedWidget>
#include <DPushButton>

inline constexpr int kMinimumWidth = { 100 };

WorkspaceWidget::WorkspaceWidget(QWidget *parent) : DWidget(parent)
{
    initUi();
}

void WorkspaceWidget::initUi()
{
    mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    setMinimumWidth(kMinimumWidth);

    stackEditWorkspaceWidget = new DStackedWidget(this);
    mainLayout->addWidget(stackEditWorkspaceWidget);
}

void WorkspaceWidget::addWorkspaceWidget(const QString &title, AbstractWidget *treeWidget, const QString &iconName)
{
    Q_UNUSED(iconName);
    auto qTreeWidget = static_cast<DWidget *>(treeWidget->qWidget());

    qTreeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    editWorkspaceWidgets.insert(title, qTreeWidget);
    stackEditWorkspaceWidget->addWidget(qTreeWidget);

    switchWidgetWorkspace(title);
}

void WorkspaceWidget::registerToolBtnToWidget(DToolButton *btn, const QString &title)
{
    if (!btn)
        return;

    toolBtnOfWidget.insert(title, btn);
}

QList<DToolButton *> WorkspaceWidget::getAllToolBtn()
{
    return toolBtnOfWidget.values();
}

QList<DToolButton *> WorkspaceWidget::getToolBtnByTitle(const QString &title)
{
    return toolBtnOfWidget.values(title);
}

void WorkspaceWidget::switchWidgetWorkspace(const QString &title)
{
    auto lastTitle = currentTitle();
    auto widget = editWorkspaceWidgets[title];
    stackEditWorkspaceWidget->setCurrentWidget(widget);

    emit expandStateChange(widget->property("canExpand").toBool());

    if (!addedToController || (lastTitle == title))
        return;

    //update toolbtn`s state at header
    for (auto btn : getToolBtnByTitle(lastTitle)) {
        toolBtnState[btn] = btn->isVisible();
        btn->setVisible(false);
    }

    for (auto btn : getToolBtnByTitle(title)) {
        if (toolBtnState.contains(btn))
            btn->setVisible(toolBtnState[btn]);
    }

    emit workSpaceWidgeSwitched(title);
}

bool WorkspaceWidget::getCurrentExpandState()
{
    return stackEditWorkspaceWidget->currentWidget()->property("canExpand").toBool();
}

QStringList WorkspaceWidget::allWidgetTitles() const
{
    return editWorkspaceWidgets.keys();
}

QString WorkspaceWidget::currentTitle() const
{
    auto currentWidget = stackEditWorkspaceWidget->currentWidget();
    return editWorkspaceWidgets.key(currentWidget);
}
