// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "singlechoicebox.h"

#include <DRadioButton>

#include <QVBoxLayout>
#include <QSet>

DWIDGET_USE_NAMESPACE

class SingleChoiceBoxPrivate
{
    friend class SingleChoiceBox;
    QList<DRadioButton*> radioButtons{};
    QGroupBox *groupBox = nullptr;
    QVBoxLayout *vLayoutMain = nullptr;
    QVBoxLayout *vLayoutCentral = nullptr;
    inline SingleChoiceBoxPrivate(){}
};

SingleChoiceBox::SingleChoiceBox(QWidget *parent)
    : DWidget (parent)
    , d(new SingleChoiceBoxPrivate)
{
    d->groupBox = new QGroupBox();
    d->vLayoutMain = new QVBoxLayout();
    d->vLayoutCentral = new QVBoxLayout();
    d->groupBox->setLayout(d->vLayoutMain);
    d->vLayoutCentral->addWidget(d->groupBox);
    setLayout(d->vLayoutCentral);
}

SingleChoiceBox::~SingleChoiceBox()
{
    if (d) {
        delete d;
    }
}

void SingleChoiceBox::setInfos(QSet<SingleChoiceBox::Info> infos)
{
    for (auto checkbox : d->radioButtons) {
        delete checkbox;
    }

    for (auto info : infos) {
        auto radioButton = new DRadioButton();
        QObject::connect(radioButton, &DRadioButton::toggled,
                         this, &SingleChoiceBox::toggled,Qt::UniqueConnection);
        d->radioButtons << radioButton;
        radioButton->setIcon(info.icon);
        radioButton->setText(info.text);
        radioButton->setToolTip(info.tooltip);
        d->vLayoutMain->addWidget(radioButton);
    }
}

void SingleChoiceBox::setChoiceTitle(const QString &title)
{
    d->groupBox->setTitle(title);
}

void SingleChoiceBox::toggled(bool checked)
{
    auto toggledIns = qobject_cast<DRadioButton*>(sender());
    if (toggledIns && checked) {
        Info info { toggledIns->text(),
                    toggledIns->toolTip(),
                    toggledIns->icon() };
        emit selected(info);
    }
}

uint qHash(const SingleChoiceBox::Info &info, uint seed)
{
    return qHash(info.text + " " + info.tooltip, seed);
}
