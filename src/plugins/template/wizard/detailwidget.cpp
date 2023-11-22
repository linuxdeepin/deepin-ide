// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidget.h"
#include "templateparser.h"

#include "common/util/custompaths.h"

#include <DLabel>
#include <DComboBox>
#include <DPushButton>
#include <DMessageBox>
#include <DFileDialog>
#include <DLineEdit>
#include <DWidget>

#include <QVBoxLayout>
#include <QPainter>

DWIDGET_USE_NAMESPACE

class DetailWidgetPrivate
{
    friend class DetailWidget;

    QString templatePath;
    QMap<QString, DLineEdit*> lineEditMap;
    QMap<QString, DComboBox*> comboBoxMap;
    WizardInfo wizardInfo;
};

DetailWidget::DetailWidget(DWidget *parent)
    : DScrollArea(parent)
    , d(new DetailWidgetPrivate())
{

}

DetailWidget::DetailWidget(const QString &templatePath, DWidget *parent)
    : DScrollArea(parent)
    , d(new DetailWidgetPrivate())
{
    d->templatePath = templatePath;
    if (!TemplateParser::readWizardConfig(d->templatePath, d->wizardInfo))
        return;

    DWidget *widget = new DWidget();
    QVBoxLayout *vLayout = new QVBoxLayout();
    widget->setLayout(vLayout);

    DLabel *titleLabel = new DLabel(d->wizardInfo.trDisplayName);
    vLayout->addWidget(titleLabel);
    vLayout->addSpacing(10);

    auto iter = d->wizardInfo.configures.begin();
    for (; iter != d->wizardInfo.configures.end(); ++iter) {
        QHBoxLayout *hLayout = new QHBoxLayout();
        DLabel *label = new DLabel(iter->displayName + ":");
        label->setFixedSize(120, 30);
        hLayout->addWidget(label, 0, Qt::AlignLeft);
        hLayout->setStretchFactor(label, 1);

        if ("lineEdit" == iter->type) {
            DLineEdit *lineEdit = new DLineEdit();
            if (!iter->defaultValues.isEmpty()) {
                lineEdit->setText(iter->defaultValues.at(0));
            }
            hLayout->addWidget(lineEdit, 0, Qt::AlignLeft);
            hLayout->setStretchFactor(lineEdit, 3);

            d->lineEditMap.insert(iter->key, lineEdit);
            if (iter->browse) {
                lineEdit->setFixedSize(300, 30);
                lineEdit->lineEdit()->setReadOnly(true);

                DPushButton *browse = new DPushButton(tr("Browse..."));
                browse->setFixedSize(100, 30);
                hLayout->addWidget(browse, 0, Qt::AlignRight);
                hLayout->setStretchFactor(browse, 1);

                connect(browse, &DPushButton::clicked, [=]() {
                    QString path = DFileDialog::getExistingDirectory(this, tr("Choose path"), QDir::homePath());
                    if (!path.isEmpty()) {
                        lineEdit->setText(path);
                    }
                });
            } else {
                lineEdit->setFixedSize(400, 30);
            }
        } else if ("comboBox" == iter->type) {
            DComboBox *comboBox = new DComboBox();
            comboBox->setFixedSize(400, 30);
            hLayout->addWidget(comboBox, 0, Qt::AlignLeft);
            hLayout->setStretchFactor(comboBox, 3);

            if (!iter->defaultValues.isEmpty()) {
                comboBox->addItems(iter->defaultValues);
                comboBox->setCurrentIndex(0);
            }
            d->comboBoxMap.insert(iter->key, comboBox);
        }

        vLayout->addLayout(hLayout);
    }

    setWidget(widget);
}

DetailWidget::~DetailWidget()
{
    if (d)
        delete d;
}

bool DetailWidget::getGenParams(PojectGenParam &param)
{
    foreach (auto key, d->lineEditMap.keys()) {
        auto lineEdit = d->lineEditMap.value(key);

        if (lineEdit->text().trimmed().isEmpty()) {
            DMessageBox::critical(this, "tip", "The value of " + key + " is empty");
            return false;
        }

        param.settingParamMap.insert("%{" + key + "}", lineEdit->text().trimmed());
    }

    foreach (auto key, d->comboBoxMap.keys()) {
        auto comboBox = d->comboBoxMap.value(key);
        param.settingParamMap.insert("%{" + key + "}", comboBox->currentText().trimmed());
    }

    param.templatePath = CustomPaths::global(CustomPaths::Templates)
            + QDir::separator() + d->templatePath;
    param.generator = d->wizardInfo.generator;
    if (d->wizardInfo.type == "project") {
        param.type = Project;
    } else if (d->wizardInfo.type == "file") {
        param.type = File;
    }
    param.kit = d->wizardInfo.kit;
    param.language = d->wizardInfo.language;

    return true;
}



