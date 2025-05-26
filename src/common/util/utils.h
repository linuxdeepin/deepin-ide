// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <DToolButton>

#include <QAction>
#include <QApplication>

DWIDGET_USE_NAMESPACE
namespace utils {
    
    static DToolButton* createIconButton(QAction *action, QWidget *parent) {
        DToolButton *iconBtn = new DToolButton(parent);
        iconBtn->setFocusPolicy(Qt::NoFocus);
        iconBtn->setEnabled(action->isEnabled());
        iconBtn->setIcon(action->icon());
        iconBtn->setFixedSize(QSize(36, 36));

        QString toolTipStr = action->text();
        if (!action->shortcut().isEmpty()) {
            toolTipStr = toolTipStr + " " + action->shortcut().toString();
        }

        if (!toolTipStr.isEmpty())
            iconBtn->setToolTip(toolTipStr);

        QObject::connect(iconBtn, &DToolButton::clicked, action, &QAction::triggered);
        QObject::connect(action, &QAction::changed, iconBtn, [=] {
            QString toolTipStr = action->text() + " " + action->shortcut().toString();
            iconBtn->setToolTip(toolTipStr);
            iconBtn->setIcon(action->icon());
            iconBtn->setEnabled(action->isEnabled());
        });

        return iconBtn;
    }

    static bool isWayland()
    {
        return QApplication::platformName() == "wayland";
    }
}

#endif
