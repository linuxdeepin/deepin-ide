// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profilesettingwidget.h"
#include "common/common.h"

#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

class ProfileSettingWidgetPrivate
{
    friend class ProfileSettingWidget;
    QVBoxLayout *vLayout = nullptr;
    QHBoxLayout *hlayout = nullptr;
    QLabel *languageEdit = nullptr;
    QComboBox *cbChooseLanguage = nullptr;
    LanguagePaths languagePaths;
};

ProfileSettingWidget::ProfileSettingWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new ProfileSettingWidgetPrivate)
{
    readTranslate();
    setupUi();
    readConfig();
}

ProfileSettingWidget::~ProfileSettingWidget()
{
    if(d)
        delete d;
}

QString ProfileSettingWidget::translateFilePath()
{
    return CustomPaths::global(CustomPaths::Flags::Configures)
            + QDir::separator() + QString("translate.support");
}

QString ProfileSettingWidget::languageFilePath()
{
    return CustomPaths::user(CustomPaths::Flags::Configures)
            + QDir::separator() + QString("chooselanguage.support");
}

const LanguagePaths &ProfileSettingWidget::getLanguagePaths() const
{
    return d->languagePaths;
}

void ProfileSettingWidget::saveConfig()
{
    QFile file(languageFilePath());
    QTextStream txtInput(&file);
    QString chooseFileName = d->languagePaths.value(d->cbChooseLanguage->currentText());
    QString currentFileName;
    if (file.open(QIODevice::ReadOnly)) {
        currentFileName = txtInput.readLine();
        file.close();
    }
    if (chooseFileName == currentFileName) {
        return;
    }

    if (file.open(QFile::WriteOnly)) {

        file.write(chooseFileName.toUtf8());
        file.close();
    }
    QMessageBox msgBox;
    QPushButton *okButton = new QPushButton(tr("Ok"));
    msgBox.addButton(okButton, QMessageBox::ButtonRole::NoRole);
    msgBox.setWindowTitle(tr("Restart Required--deep-in unioncode"));
    msgBox.setText(tr("The language change will take effect after restart."));
    msgBox.exec();
}

void ProfileSettingWidget::readConfig()
{
    QFile file(languageFilePath());
    QTextStream txtInput(&file);
    QString fileName;
    if (file.open(QIODevice::ReadOnly)) {
        fileName = txtInput.readLine();
        file.close();
    }
    d->cbChooseLanguage->setCurrentIndex(0);
    for (int i = 0; i < d->cbChooseLanguage->count(); i++) {
        if (d->languagePaths.value(d->cbChooseLanguage->itemText(i)) == fileName) {
            d->cbChooseLanguage->setCurrentIndex(i);
            break;
        }
    }
}

void ProfileSettingWidget::setupUi()
{
    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    this->setLayout(d->vLayout);

    if (!d->hlayout)
        d->hlayout = new QHBoxLayout();

    if (!d->languageEdit) {
        d->languageEdit = new QLabel(tr("language:"));
    }

    if (!d->cbChooseLanguage)
        d->cbChooseLanguage = new QComboBox();
    d->cbChooseLanguage->setFixedWidth(200);

    QStringList nameList = d->languagePaths.keys();
    int i = 0;
    for (auto name : nameList) {
        d->cbChooseLanguage->insertItem(i, name);
        i++;
    }

    d->hlayout->setMargin(10);
    d->hlayout->setSpacing(10);
    d->hlayout->addWidget(d->languageEdit);
    d->hlayout->addWidget(d->cbChooseLanguage, 5, Qt::AlignmentFlag::AlignLeft);

    d->vLayout->setAlignment(Qt::AlignmentFlag::AlignTop);
    d->vLayout->addLayout(d->hlayout);
}

void ProfileSettingWidget::readTranslate()
{
    QFile file(translateFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array = doc.array();
        for (auto node : array) {
            auto obj = node.toObject();
            QJsonValue nameVal = obj.value(lNameItem);
            QString name = nameVal.toString();

            QJsonValue pathVal = obj.value(lPathItem);
            QString path = pathVal.toString();
            d->languagePaths.insert(name, path);
        }
    }
}
