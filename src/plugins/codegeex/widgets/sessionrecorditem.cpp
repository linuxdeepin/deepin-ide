// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionrecorditem.h"

#include <DPushButton>
#include <DToolButton>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

SessionRecordItem::SessionRecordItem(QWidget *parent)
    : DWidget (parent)
{
    initUI();
    initConnection();
}

void SessionRecordItem::updateItem(const RecordData &data)
{
    talkId = data.talkId;

//    if (deleteButton)
//        deleteButton->setEnabled(!talkId.isEmpty());

    if (promotLabel)
        promotLabel->setText(data.promot);

    if (dateLabel)
        dateLabel->setText(data.date);
}

void SessionRecordItem::onDeleteButtonClicked()
{
    CodeGeeXManager::instance()->deleteSession(talkId);
}

void SessionRecordItem::onRecordClicked()
{
    CodeGeeXManager::instance()->sendMessage(promotLabel->text());
    Q_EMIT closeHistoryWidget();
}

void SessionRecordItem::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    recordButton = new DPushButton(this);
    recordButton->setFixedHeight(64);
    layout->addWidget(recordButton);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setMargin(10);
    btnLayout->setSpacing(10);

    recordButton->setLayout(btnLayout);

    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->setMargin(0);
    textLayout->setSpacing(5);

    promotLabel = new DLabel(recordButton);
    textLayout->addWidget(promotLabel);

    dateLabel = new DLabel(recordButton);
    textLayout->addWidget(dateLabel);

    btnLayout->addLayout(textLayout);

    deleteButton = new DToolButton(this);
    deleteButton->setFixedSize(QSize(30, 30));
    deleteButton->setIcon(QIcon::fromTheme("codegeex_clear"));
    btnLayout->addWidget(deleteButton, 0);
}

void SessionRecordItem::initConnection()
{
    connect(recordButton, &DPushButton::clicked, this, &SessionRecordItem::onRecordClicked);
    connect(deleteButton, &DPushButton::clicked, this, &SessionRecordItem::onDeleteButtonClicked);
}
