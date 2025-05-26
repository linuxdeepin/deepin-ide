// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buildmanager.h"
#include "base/abstractaction.h"
#include "common/widget/outputpane.h"
#include "common/util/commandparser.h"
#include "common/project/projectinfo.h"
#include "common/find/outputdocumentfind.h"
#include "common/actionmanager/actioncontainer.h"
#include "problemoutputpane.h"
#include "commonparser.h"
#include "transceiver/buildersender.h"
#include "compileoutputpane.h"
#include "tasks/taskmodel.h"
#include "common/util/utils.h"
#include "settingdialog.h"

#include "services/builder/builderservice.h"
#include "services/editor/editorservice.h"
#include "base/abstractoutputparser.h"
#include "services/window/windowservice.h"
#include "services/builder/buildergenerator.h"
#include "services/option/optionmanager.h"
#include "services/project/projectservice.h"
#include "services/language/languageservice.h"

#include <DGuiApplicationHelper>
#include <DComboBox>
#include <DFrame>
#include <DDialog>

#include <QSplitter>
#include <QCoreApplication>

using namespace dpfservice;

class BuildManagerPrivate
{
    friend class BuildManager;

    QAction *buildAction;
    QAction *rebuildAction;
    QAction *cleanAction;
    QAction *cancelAction;

    DToolButton *buildCancelBtn = nullptr;
    CompileOutputPane *compileOutputPane = nullptr;
    ProblemOutputPane *problemOutputPane = nullptr;
    DWidget *issuesWidget = nullptr;
    DWidget *compileOutputWidget = nullptr;

    DWidget *compileWidget = nullptr;

    QString activedKitName;
    QString activedWorkingDir;

    std::unique_ptr<AbstractOutputParser> outputParser = nullptr;

    QProcess cmdProcess;
    QFuture<void> buildThread;

    BuildState currentState = BuildState::kNoBuild;
};

BuildManager *BuildManager::instance()
{
    static BuildManager ins;
    return &ins;
}

BuildManager::BuildManager(QObject *parent)
    : QObject(parent), d(new BuildManagerPrivate())
{
    addMenu();
    initCompileWidget();

    d->outputParser.reset(new CommonParser());
    connect(d->outputParser.get(), &AbstractOutputParser::addOutput, this, &BuildManager::addOutput, Qt::DirectConnection);
    connect(d->outputParser.get(), &AbstractOutputParser::addTask, d->problemOutputPane, &ProblemOutputPane::addTask, Qt::DirectConnection);

    QObject::connect(this, &BuildManager::sigOutputCompileInfo, this, &BuildManager::slotOutputCompileInfo);
    QObject::connect(this, &BuildManager::sigOutputProblemInfo, this, &BuildManager::slotOutputProblemInfo);

    qRegisterMetaType<BuildState>("BuildState");
    qRegisterMetaType<BuildCommandInfo>("BuildCommandInfo");
    QObject::connect(this, &BuildManager::sigBuildState, this, &BuildManager::slotBuildState);
    QObject::connect(this, &BuildManager::sigOutputNotify, this, &BuildManager::slotOutputNotify);
    QObject::connect(this, &BuildManager::sigResetBuildUI, this, &BuildManager::slotResetBuildUI);
}

BuildManager::~BuildManager()
{
    if (d) {
        delete d;
    }
}

void BuildManager::addMenu()
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    auto actionInit = [&](QAction *action, const QString &actionID,
                          const QKeySequence &key,
                          const QString &iconFileName = {})
            -> Command * {
        if (!iconFileName.isEmpty())
            action->setIcon(QIcon::fromTheme(iconFileName));
        auto cmd = ActionManager::instance()->registerAction(action, actionID);
        if (!key.isEmpty())
            cmd->setDefaultKeySequence(key);
        return cmd;
    };

    auto mBuild = ActionManager::instance()->actionContainer(M_BUILD);
    d->buildAction = new QAction(MWMBA_BUILD, this);
    auto cmd = actionInit(d->buildAction, "Build.Build",
                          QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B),
                          "build");
    mBuild->addAction(cmd);
    d->buildCancelBtn = windowService->addTopToolItem(cmd, true, Priority::low);

    d->cancelAction = new QAction(MWMBA_CANCEL, this);
    d->cancelAction->setIcon(QIcon::fromTheme("cancel"));
    cmd = actionInit(d->cancelAction, "Build.Cancel", QKeySequence(Qt::ALT | Qt::Key_Backspace));
    mBuild->addAction(cmd);

    d->rebuildAction = new QAction(MWMBA_REBUILD, this);
    d->rebuildAction->setIcon(QIcon::fromTheme("rebuild"));
    actionInit(d->rebuildAction, "Build.Rebuild", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B));

    d->cleanAction = new QAction(MWMBA_CLEAN, this);
    d->cleanAction->setIcon(QIcon::fromTheme("clearall"));
    actionInit(d->cleanAction, "Build.Clean", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));

    QObject::connect(d->buildAction, &QAction::triggered,
                     this, &BuildManager::buildCancelProject, Qt::DirectConnection);
    QObject::connect(d->rebuildAction, &QAction::triggered,
                     this, &BuildManager::rebuildProject, Qt::DirectConnection);
    QObject::connect(d->cleanAction, &QAction::triggered,
                     this, &BuildManager::cleanProject, Qt::DirectConnection);
    QObject::connect(d->cancelAction, &QAction::triggered,
                     this, &BuildManager::cancelBuild, Qt::DirectConnection);
}

void BuildManager::initCompileWidget()
{
    d->compileWidget = new DWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(d->compileWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    initIssueList();
    initCompileOutput();

    QSplitter *spl = new QSplitter(Qt::Horizontal);
    spl->addWidget(d->compileOutputWidget);
    spl->addWidget(d->issuesWidget);
    spl->setHandleWidth(1);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(spl);
    if (auto holder = createFindPlaceHolder())
        mainLayout->addWidget(holder);
}

void BuildManager::initIssueList()
{
    d->problemOutputPane = new ProblemOutputPane(d->compileWidget);

    QLabel *issusListText = new QLabel(d->compileWidget);
    issusListText->setText(tr("Issues list"));
    issusListText->setContentsMargins(10, 0, 0, 0);

    DToolButton *filterButton = new DToolButton(d->compileWidget);
    filterButton->setFixedSize(26, 26);
    filterButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    filterButton->setIcon(QIcon::fromTheme("filter"));
    filterButton->setContentsMargins(0, 0, 0, 0);
    filterButton->setToolTip(tr("Filter"));

    DToolButton *settingButton = new DToolButton(d->compileWidget);
    settingButton->setFixedSize(26, 26);
    settingButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    settingButton->setIcon(QIcon::fromTheme("settings"));
    settingButton->setContentsMargins(0, 0, 0, 0);
    settingButton->setToolTip(tr("Settings"));

    DFrame *issueTopWidget = new DFrame(d->compileWidget);
    DStyle::setFrameRadius(issueTopWidget, 0);
    issueTopWidget->setLineWidth(0);
    issueTopWidget->setFixedHeight(36);
    QHBoxLayout *hIssueTopLayout = new QHBoxLayout(issueTopWidget);
    hIssueTopLayout->addWidget(issusListText);
    hIssueTopLayout->addWidget(settingButton);
    hIssueTopLayout->addWidget(filterButton);
    hIssueTopLayout->setSpacing(0);
    hIssueTopLayout->setContentsMargins(0, 0, 5, 0);
    hIssueTopLayout->setAlignment(Qt::AlignVCenter);

    DMenu *filterMenu = new DMenu(filterButton);

    QAction *showAllAction = new QAction(tr("All"), this);
    showAllAction->setCheckable(true);
    showAllAction->setChecked(true);
    filterMenu->addAction(showAllAction);

    QAction *showErrorAction = new QAction(tr("Error"), this);
    showErrorAction->setCheckable(true);
    filterMenu->addAction(showErrorAction);

    QAction *showWarningAction = new QAction(tr("Warning"), this);
    showWarningAction->setCheckable(true);
    filterMenu->addAction(showWarningAction);

    d->issuesWidget = new DWidget(d->compileWidget);
    QVBoxLayout *issuesListLayout = new QVBoxLayout(d->issuesWidget);
    issuesListLayout->setSpacing(0);
    issuesListLayout->setContentsMargins(0, 0, 0, 0);
    issuesListLayout->addWidget(issueTopWidget);
    issuesListLayout->addWidget(new DHorizontalLine(d->issuesWidget));
    issuesListLayout->addWidget(d->problemOutputPane);

    connect(filterMenu, &DMenu::triggered, [=](QAction *action) {
        if (action == showAllAction) {
            d->problemOutputPane->showSpecificTasks(ShowType::All);
            showAllAction->setChecked(true);
            showErrorAction->setChecked(false);
            showWarningAction->setChecked(false);
        } else if (action == showErrorAction) {
            d->problemOutputPane->showSpecificTasks(ShowType::Error);
            showAllAction->setChecked(false);
            showErrorAction->setChecked(true);
            showWarningAction->setChecked(false);
        } else {
            d->problemOutputPane->showSpecificTasks(ShowType::Warning);
            showAllAction->setChecked(false);
            showErrorAction->setChecked(false);
            showWarningAction->setChecked(true);
        }
    });
    connect(filterButton, &DToolButton::clicked, [=]() {
        QPoint buttonPos = filterButton->mapToGlobal(QPoint(0, filterButton->height()));
        QPoint menuPos = buttonPos + QPoint(0, 5);
        filterMenu->popup(menuPos);
    });
    connect(settingButton, &DToolButton::clicked, this, &BuildManager::showSettingDialog);
}

void BuildManager::initCompileOutput()
{
    d->compileOutputPane = new CompileOutputPane(d->compileWidget);

    QLabel *compileOutputText = new QLabel(d->compileWidget);
    compileOutputText->setText(tr("Compile Output"));
    compileOutputText->setContentsMargins(10, 0, 0, 0);

    QHBoxLayout *hOutputTopLayout = new QHBoxLayout();
    hOutputTopLayout->addWidget(compileOutputText);
    hOutputTopLayout->setSpacing(0);
    hOutputTopLayout->setContentsMargins(0, 0, 5, 0);
    hOutputTopLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    auto createVLine = [this] {
        DVerticalLine *vLine = new DVerticalLine(d->compileWidget);
        vLine->setFixedHeight(20);
        return vLine;
    };

    // init toolButton
    hOutputTopLayout->addSpacing(10);
    hOutputTopLayout->addWidget(createVLine());
    hOutputTopLayout->addSpacing(10);

    auto btn = utils::createIconButton(d->cancelAction, d->compileWidget);
    btn->setFixedSize(QSize(26, 26));
    hOutputTopLayout->addWidget(btn);
    btn = utils::createIconButton(d->rebuildAction, d->compileWidget);
    btn->setFixedSize(QSize(26, 26));
    hOutputTopLayout->addWidget(btn);
    btn = utils::createIconButton(d->cleanAction, d->compileWidget);
    btn->setFixedSize(QSize(26, 26));
    hOutputTopLayout->addWidget(btn);

    DToolButton *clearLogBtn = new DToolButton(d->compileWidget);
    clearLogBtn->setIconSize({ 16, 16 });
    clearLogBtn->setFixedSize({ 26, 26 });
    clearLogBtn->setIcon(QIcon::fromTheme("clear_log"));
    clearLogBtn->setToolTip(tr("Clear Output"));
    connect(clearLogBtn, &DToolButton::clicked, d->compileOutputPane, &CompileOutputPane::clearContents);
    hOutputTopLayout->addWidget(createVLine());
    hOutputTopLayout->addWidget(clearLogBtn);

    DFrame *OutputTopWidget = new DFrame(d->compileWidget);
    DStyle::setFrameRadius(OutputTopWidget, 0);
    OutputTopWidget->setLineWidth(0);
    OutputTopWidget->setLayout(hOutputTopLayout);
    OutputTopWidget->setFixedHeight(36);

    d->compileOutputWidget = new DWidget(d->compileWidget);
    auto outputLayout = new QVBoxLayout(d->compileOutputWidget);
    outputLayout->setContentsMargins(0, 0, 0, 0);
    outputLayout->setSpacing(0);
    outputLayout->addWidget(OutputTopWidget);
    outputLayout->addWidget(new DHorizontalLine(d->compileOutputWidget));
    outputLayout->addWidget(d->compileOutputPane);
}

QWidget *BuildManager::createFindPlaceHolder()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return nullptr;

    auto docFind = new OutputDocumentFind(d->compileOutputPane);
    return windowService->createFindPlaceHolder(d->compileOutputPane, docFind);
}

void BuildManager::buildCancelProject()
{
    if (d->currentState == BuildState::kBuilding)
        cancelBuild();
    else
        buildProject();
}

void BuildManager::buildProject()
{
    execBuildStep({ Build });
}

void BuildManager::rebuildProject()
{
    execBuildStep({ Clean, Build });
}

void BuildManager::cleanProject()
{
    execBuildStep({ Clean });
}

void BuildManager::cancelBuild()
{
    if (d->currentState == kBuilding) {
        d->buildThread.cancel();
        disconnectSignals();
        d->cmdProcess.kill();
    }
}

void BuildManager::execBuildStep(QList<BuildMenuType> menuTypelist)
{
    // save all modified files before build.
    dpfGetService(EditorService)->saveAll();

    if (!canStartBuild()) {
        QMetaObject::invokeMethod(this, "message",
                                  Q_ARG(QString, "The builder is running, please try again later!"));
        return;
    }

    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService || !projectService->getProjectInfo)
        return;

    ProjectInfo projectInfo = projectService->getProjectInfo(d->activedKitName, d->activedWorkingDir);
    if (!projectInfo.isVaild())
        return;

    auto builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        auto generator = builderService->create<BuilderGenerator>(d->activedKitName);
        if (generator) {
            emit sigResetBuildUI();
            generator->appendOutputParser(d->outputParser);
            QList<BuildCommandInfo> list;
            foreach (auto menuType, menuTypelist) {
                BuildCommandInfo info = generator->getMenuCommand(menuType, projectInfo);
                QString retMsg;
                bool ret = generator->checkCommandValidity(info, retMsg);
                if (!ret) {
                    outputLog(retMsg, OutputPane::OutputFormat::StdErr);
                    continue;
                }
                list.append(info);
            }
            execCommands(list, false);
        } else {
            auto windowService = dpfGetService(WindowService);
            windowService->notify(1, tr("Warning"), tr("The project does not have an associated build kit. Please reopen the project and select the corresponding build tool."), {});
        }
    }
}

CompileOutputPane *BuildManager::getCompileOutputPane() const
{
    return d->compileOutputPane;
}

ProblemOutputPane *BuildManager::getProblemOutputPane() const
{
    return d->problemOutputPane;
}

DWidget *BuildManager::getCompileWidget() const
{
    return d->compileWidget;
}

void BuildManager::slotResetBuildUI()
{
    d->compileOutputPane->clearContents();
    d->problemOutputPane->clearContents();

    uiController.switchContext(tr("&Build"));
}

void BuildManager::showSettingDialog()
{
    SettingDialog dlg;
    dlg.exec();
}

void BuildManager::setActivatedProjectInfo(const QString &kitName, const QString &workingDir)
{
    d->activedKitName = kitName;
    d->activedWorkingDir = workingDir;
    auto service = dpfGetService(LanguageService);
    if (service) {
        auto generator = service->create<LanguageGenerator>(kitName);
        if (generator && generator->isNeedBuild())
            slotBuildState(BuildState::kNoBuild);
        else
            slotBuildState(BuildState::kBuildNotSupport);
    }
}

void BuildManager::clearActivatedProjectInfo()
{
    d->activedKitName.clear();
    d->activedWorkingDir.clear();
}

bool BuildManager::isActivatedProject(const ProjectInfo &info)
{
    if (info.kitName() == d->activedKitName && info.workspaceFolder() == d->activedWorkingDir)
        return true;
    return false;
}

bool BuildManager::handleCommand(const QList<BuildCommandInfo> &commandInfo, bool isSynchronous)
{
    if (!canStartBuild()) {
        QMetaObject::invokeMethod(this, "message",
                                  Q_ARG(QString, "The builder is running, please try again later!"));
        return false;
    }

    auto &ctx = dpfInstance.serviceContext();
    auto builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        auto generator = builderService->create<BuilderGenerator>(commandInfo.at(0).kitName);
        if (generator) {
            emit sigResetBuildUI();
            generator->appendOutputParser(d->outputParser);
            QString retMsg;
            bool ret = generator->checkCommandValidity(commandInfo.at(0), retMsg);
            if (!ret) {
                outputLog(retMsg, OutputPane::OutputFormat::StdErr);
                return false;
            }
        }
        execCommands(commandInfo, isSynchronous);
    }
    return true;
}

bool BuildManager::execCommands(const QList<BuildCommandInfo> &commandList, bool isSynchronous)
{
    //Synchronous execution is required in commandLine build model
    if (isSynchronous) {
        if (!commandList.isEmpty()) {
            for (auto command : commandList) {
                execCommand(command);
            }
        }
    } else {
        if (!commandList.isEmpty()) {
            d->buildThread = QtConcurrent::run([=]() {
                QMutexLocker locker(&releaseMutex);
                for (auto command : commandList) {
                    execCommand(command);
                }
            });
        }
    }

    return true;
}

bool BuildManager::execCommand(const BuildCommandInfo &info)
{
    outBuildState(BuildState::kBuilding);
    bool ret = false;
    QString executeResult = tr("Execute command failed!\n");

    d->cmdProcess.setWorkingDirectory(info.workingDir);

    QString startMsg = tr("Start execute command: \"%1\" \"%2\" in workspace \"%3\".\n")
                               .arg(info.program, info.arguments.join(" "), info.workingDir);
    outputLog(startMsg, OutputPane::OutputFormat::NormalMessage);

    connect(&d->cmdProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
                if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
                    ret = true;
                    executeResult = tr("The process \"%1\" exited normally.\n").arg(d->cmdProcess.program());
                } else if (exitStatus == QProcess::NormalExit) {
                    ret = false;
                    executeResult = tr("The process \"%1\" exited with code %2.\n")
                                            .arg(d->cmdProcess.program(), QString::number(exitcode));
                } else {
                    ret = false;
                    executeResult = tr("The process \"%1\" crashed.\n")
                                            .arg(d->cmdProcess.program());
                }
            });

    connect(&d->cmdProcess, &QProcess::readyReadStandardOutput, [&]() {
        d->cmdProcess.setReadChannel(QProcess::StandardOutput);
        while (d->cmdProcess.canReadLine()) {
            QString line = QString::fromUtf8(d->cmdProcess.readLine());
            outputLog(line, OutputPane::OutputFormat::StdOut);
        }
    });

    connect(&d->cmdProcess, &QProcess::readyReadStandardError, [&]() {
        d->cmdProcess.setReadChannel(QProcess::StandardError);
        while (d->cmdProcess.canReadLine()) {
            QString line = QString::fromUtf8(d->cmdProcess.readLine());
            outputLog(line, OutputPane::OutputFormat::StdErr);
            outputError(line);
        }
    });

    d->cmdProcess.start(info.program, info.arguments);
    d->cmdProcess.waitForFinished(-1);

    disconnectSignals();
    outputLog(executeResult, ret ? OutputPane::OutputFormat::NormalMessage : OutputPane::OutputFormat::StdErr);

    QString endMsg = tr("Execute command finished.\n");
    outputLog(endMsg, OutputPane::OutputFormat::NormalMessage);

    BuildState buildState = ret ? BuildState::kNoBuild : BuildState::kBuildFailed;
    outBuildState(buildState);

    outputNotify(buildState, info);
    return ret;
}

void BuildManager::outputLog(const QString &content, const OutputPane::OutputFormat format)
{
    emit sigOutputCompileInfo(content, format);
}

void BuildManager::outputError(const QString &content)
{
    emit sigOutputProblemInfo(content);
}

void BuildManager::outputNotify(const BuildState &state, const BuildCommandInfo &commandInfo)
{
    emit sigOutputNotify(state, commandInfo);
}

void BuildManager::slotOutputCompileInfo(const QString &content, const OutputPane::OutputFormat format)
{
    if (format == OutputPane::OutputFormat::StdOut || OutputPane::OutputFormat::NormalMessage) {
        std::cout << content.toStdString() << std::endl;
    } else if (format == OutputPane::OutputFormat::StdErr) {
        std::cerr << content.toStdString() << std::endl;
    }
    d->outputParser->stdOutput(content, format);
}

void BuildManager::slotOutputProblemInfo(const QString &content)
{
    d->outputParser->stdError(content);
}

void BuildManager::slotOutputNotify(const BuildState &state, const BuildCommandInfo &commandInfo)
{
    BuilderSender::notifyBuildState(state, commandInfo);
}

void BuildManager::addOutput(const QString &content, const OutputPane::OutputFormat format)
{
    QString newContent = content;
    if (OutputPane::OutputFormat::NormalMessage == format
        || OutputPane::OutputFormat::ErrorMessage == format
        || OutputPane::OutputFormat::StdOut == format) {

        QDateTime curDatetime = QDateTime::currentDateTime();
        QString time = curDatetime.toString("hh:mm:ss");
        newContent = time + ": " + newContent;
    }
    d->compileOutputPane->appendText(newContent, format);
}

void BuildManager::outBuildState(const BuildState &buildState)
{
    emit sigBuildState(buildState);
}

void BuildManager::slotBuildState(const BuildState &buildState)
{
    d->currentState = buildState;

    switch (buildState) {
    case BuildState::kNoBuild:
    case BuildState::kBuildFailed: {
        d->buildCancelBtn->setEnabled(true);
        d->buildCancelBtn->setIcon(QIcon::fromTheme("build"));
        auto cmd = ActionManager::instance()->command("Build.Build");
        auto toolTip = QString(MWMBA_CANCEL).append(" %1").arg(Command::keySequencesToNativeString(cmd->keySequences()).join(" | "));
        d->buildCancelBtn->setToolTip(toolTip);
        d->rebuildAction->setEnabled(true);
        d->cleanAction->setEnabled(true);
        d->buildAction->setEnabled(true);
        d->cancelAction->setEnabled(false);
    } break;
    case BuildState::kBuilding: {
        d->buildCancelBtn->setEnabled(true);
        d->buildCancelBtn->setIcon(QIcon::fromTheme("cancel"));
        auto cmd = ActionManager::instance()->command("Build.Cancel");
        auto toolTip = QString(MWMBA_CANCEL).append(" %1").arg(Command::keySequencesToNativeString(cmd->keySequences()).join(" | "));
        d->buildCancelBtn->setToolTip(toolTip);
        d->rebuildAction->setEnabled(false);
        d->cleanAction->setEnabled(false);
        d->buildAction->setEnabled(true);
        d->cancelAction->setEnabled(true);
    } break;
    case BuildState::kBuildNotSupport: {
        d->buildCancelBtn->setEnabled(false);
        d->rebuildAction->setEnabled(false);
        d->cleanAction->setEnabled(false);
        d->buildAction->setEnabled(false);
        d->cancelAction->setEnabled(false);
    } break;
    }
}

bool BuildManager::canStartBuild()
{
    return BuildState::kBuilding != d->currentState;
}

void BuildManager::disconnectSignals()
{
    disconnect(&d->cmdProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), nullptr, nullptr);
    disconnect(&d->cmdProcess, &QProcess::readyReadStandardOutput, nullptr, nullptr);
    disconnect(&d->cmdProcess, &QProcess::readyReadStandardError, nullptr, nullptr);
}
