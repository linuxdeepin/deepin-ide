// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLEGENERATOR_H
#define GRADLEGENERATOR_H

#include "services/language/languagegenerator.h"
#include "dap/protocol.h"

class GradleGeneratorPrivate;
class GradleGenerator : public dpfservice::LanguageGenerator
{
    Q_OBJECT
public:
    explicit GradleGenerator();
    virtual ~GradleGenerator() override;

    static QString toolKitName() { return "gradle"; }

    QString debugger() override;
    bool isNeedBuild() override;
    bool isTargetReady() override;
    bool isAnsyPrepareDebug() override;
    bool prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool requestDAPPort(const QString &ppid, const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool isLaunchNotAttach() override;
    dap::LaunchRequest launchDAP(const QMap<QString, QVariant> &param) override;
    QString build(const QString& projectPath) override;
    bool isRestartDAPManually() override;
    bool isStopDAPManually() override;
    QMap<QString, QVariant> getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                              const QString &currentFile) override;
    dpfservice::RunCommandInfo getRunArguments(const dpfservice::ProjectInfo &projectInfo,
                                                         const QString &currentFile) override;

signals:

private slots:

private:
    GradleGeneratorPrivate *const d;
};

#endif // GRADLEGENERATOR_H
