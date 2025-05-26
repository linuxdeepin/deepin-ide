// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIRECTORYPROJECTGENERATOR_H
#define DIRECTORYPROJECTGENERATOR_H

#include "projectgenerator.h"
class DirectoryGeneratorPrivate;
namespace dpfservice {

class DirectoryGenerator : public ProjectGenerator
{
    Q_OBJECT
    DirectoryGeneratorPrivate *const d;

public:
    explicit DirectoryGenerator();
    virtual ~DirectoryGenerator();
    static QString toolKitName() { return "directory"; }
    virtual QString configureKitName() { return "directory"; }
    virtual DWidget *configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual void acceptConfigure() override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem *root) override;

protected:
    dpfservice::ProjectInfo prjInfo;
public slots:
    void projectItemsCreated(QList<QStandardItem *> itemList);
    void handleItemUpdated(const QString &path);
};

}
#endif   // DIRECTORYPROJECTGENERATOR_H
