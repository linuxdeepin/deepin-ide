// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLSMANAGER_H
#define BINARYTOOLSMANAGER_H

#include "constants.h"
#include "common/widget/appoutputpane.h"

#include <QMenu>
#include <QProcess>
#include <QThread>
#include <QMutex>

namespace dpfservice {
class WindowService;
class TerminalService;
}
class AbstractAction;

class ToolProcess : public QObject
{
    Q_OBJECT
public:
    ToolProcess() = default;

    void setId(const QString &id) { this->id = id; }
    void setProgram(const QString &program) { this->program = program; }
    void setArguments(const QStringList &arguments) { this->arguments = arguments; }
    void setWorkingDirectory(const QString &dir) { workingDir = dir; }
    void setProcessEnvironment(const QProcessEnvironment &environment) { this->environment = environment; }

    QString readAllStandardOutput();
    QString readAllStandardError();

public Q_SLOTS:
    void start(const QString &id);
    void stop();

Q_SIGNALS:
    void finished(const QString &id, int exitCode, QProcess::ExitStatus exitStatus);
    void readyReadStandardOutput(const QString &id);
    void readyReadStandardError(const QString &id);

private:
    QString id;
    QString program;
    QStringList arguments;
    QString workingDir;
    QProcessEnvironment environment;

    QMutex mutex;
    QString stdOut;
    QString stdError;
    QProcess process;
};

class BinaryToolsManager : public QObject
{
    Q_OBJECT
public:
    static BinaryToolsManager *instance();

    void save();
    using BinaryTools = QMap<QString, QList<ToolInfo>>;
    void setTools(const BinaryTools &dataMap);
    ToolInfo findTool(const QString &id);
    ToolInfo findTool(const QString &id, const BinaryTools &tools);
    BinaryTools tools();

    void executeTool(const QString &id);
    void checkAndAddToToolbar(const BinaryTools &tools);
    void updateToolMenu(const BinaryTools &tools);
    void setToolMenu(QMenu *menu);
    void installTool(const QString &id);
    void eventTriggered(EventType event, const QVariantList &args);

Q_SIGNALS:
    void execute(const QString &id);

private Q_SLOTS:
    void executeFinished(const QString &id, int exitCode, QProcess::ExitStatus exitStatus);
    void handleReadOutput(const QString &id);
    void handleReadError(const QString &id);

private:
    explicit BinaryToolsManager(QObject *parent = nullptr);
    ~BinaryToolsManager();

    BinaryTools loadConfig(const QString &conf, QString &version);
    QStringList updateToolList();
    BinaryTools mergeTools(const BinaryTools &defTools, const BinaryTools &localTools);
    QSharedPointer<ToolProcess> createToolProcess(const ToolInfo &tool);
    bool checkCommandExists(const QString &command);
    void toolMissingHint(const ToolInfo &tool);
    void addToToolBar(const ToolInfo &tool);
    void printOutput(const QString &id, const QString &content, OutputPane::OutputFormat format);
    void stopTool(const QString &id);

private:
    QMap<QString, std::tuple<QSharedPointer<ToolProcess>, QSharedPointer<QThread>>> toolTaskMap;

    BinaryTools allTools;
    dpfservice::WindowService *windowSrv { nullptr };
    dpfservice::TerminalService *terminalSrv { nullptr };
    QMap<QString, AbstractAction *> actMap;
    QMenu *toolMenu { nullptr };
    QString cfgVersion;
};

#endif   // BINARYTOOLSMANAGER_H
