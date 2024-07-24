// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockheader.h"

#include <DStyle>
#include <DPushButton>
#include <DMenu>

#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class DockHeaderPrivate
{
    friend class DockHeader;

    QHBoxLayout *mainLayout { nullptr };
    QLabel *headerName { nullptr };
    DPushButton *select { nullptr };
};

DockHeader::DockHeader(QWidget *parent)
    : DWidget(parent), d(new DockHeaderPrivate)
{
    setAutoFillBackground(true);
    setBackgroundRole(DPalette::Base);
    d->headerName = new QLabel(this);
    d->headerName->setContentsMargins(0, 0, 0, 0);

    d->select = new DPushButton(this);
    d->select->setFlat(true);
    d->select->setIcon(QIcon::fromTheme("go-down"));
    d->select->setFixedSize(16, 16);
    d->select->setIconSize(QSize(12, 12));
    d->select->hide();

    d->mainLayout = new QHBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 5);
    d->mainLayout->setSpacing(0);
    d->mainLayout->setAlignment(Qt::AlignRight);

    auto nameLayout = new QHBoxLayout;
    nameLayout->setSpacing(4);
    nameLayout->addWidget(d->headerName);
    nameLayout->addWidget(d->select);

    d->mainLayout->addLayout(nameLayout, 1);
    d->mainLayout->setAlignment(nameLayout, Qt::AlignLeft);
}

DockHeader::~DockHeader()
{
    if (d)
        delete d;
}

void DockHeader::addToolButton(DToolButton *btn)
{
    btn->setIconSize(QSize(16, 16));
    btn->setFixedSize(QSize(26, 26));
    DStyle::setFrameRadius(btn, 6);
    d->mainLayout->insertWidget(1, btn);
}

void DockHeader::setHeaderName(const QString &headerName)
{
    d->headerName->setText(headerName);
    QFont font = d->headerName->font();
    font.setWeight(QFont::Bold);
    d->headerName->setFont(font);
}

void DockHeader::setHeaderNames(const QList<QAction *> &headers)
{
    setHeaderName(headers.first()->text());
    DMenu *menu = new DMenu(this);
    menu->addActions(headers);
    d->select->show();
    connect(d->select, &DPushButton::clicked, this, [=]() {
        auto action = menu->exec(mapToGlobal(geometry().bottomLeft()));
        if (action)
            setHeaderName(action->text());
    }, Qt::UniqueConnection);
}
