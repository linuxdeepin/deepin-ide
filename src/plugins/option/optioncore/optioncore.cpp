// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optioncore.h"
#include "mainframe/optiondefaultkeeper.h"
#include "mainframe/optiongeneralgenerator.h"

#include "common/common.h"
#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionservice.h"

#include "framework/listener/listener.h"

using namespace dpfservice;
void OptionCore::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(OptionService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool OptionCore::start()
{
    toolchains::generatGlobalFile();

    auto optionDialog = OptionDefaultKeeper::getOptionDialog();
    if (!optionDialog) {
        qCritical() << "Failed, can't init option dialog!";
        abort();
    }

    optionDialog->setModal(true);

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    if (optionService) {
        optionService->implGenerator<OptionGeneralGenerator>(OptionGeneralGenerator::kitName());
        auto generator = optionService->createGenerator<OptionGeneralGenerator>(OptionGeneralGenerator::kitName());
        if (generator) {
            auto pageWidget = dynamic_cast<PageWidget*>(generator->optionWidget());
            if (pageWidget) {
                optionDialog->insertOptionPanel(OptionGeneralGenerator::kitName(), pageWidget);
            }
        }
    }

    if (windowService && windowService->addAction && windowService->addToolBarActionItem) {
        auto actionOptions = new QAction(MWMTA_OPTIONS);
        auto actionOptionsNoIcon = new QAction(MWMTA_OPTIONS);
        ActionManager::getInstance()->registerAction(actionOptions,
                                                     "Tools.Options",
                                                     MWMTA_OPTIONS,
                                                     QKeySequence(Qt::Modifier::CTRL |
                                                                  Qt::Modifier::SHIFT |
                                                                  Qt::Key::Key_H),
                                                     ":/optioncore/images/setting.png");
        ActionManager::getInstance()->registerAction(actionOptions,
                                                     "Tools.Options",
                                                     MWMTA_OPTIONS,
                                                     QKeySequence(Qt::Modifier::CTRL |
                                                                  Qt::Modifier::SHIFT |
                                                                  Qt::Key::Key_H),
                                                     QString());
        windowService->addAction(MWM_TOOLS, new AbstractAction(actionOptionsNoIcon));
        windowService->addToolBarActionItem("Options", actionOptions, "Option.End");

        QObject::connect(actionOptions, &QAction::triggered,
                         optionDialog, &QDialog::show);
        QObject::connect(actionOptionsNoIcon, &QAction::triggered,
                         optionDialog, &QDialog::show);
    }

    DPF_USE_NAMESPACE
    QObject::connect(&Listener::instance(), &Listener::pluginsStarted, [=](){
        if (optionDialog) {
            auto list = optionService->supportGeneratorName<OptionGenerator>();
            for (auto name : list) {
                if (0 == name.compare(OptionGeneralGenerator::kitName()))
                    continue;
                auto generator = optionService->createGenerator<OptionGenerator>(name);
                if (generator) {
                    PageWidget *optionWidget = dynamic_cast<PageWidget*>(generator->optionWidget());
                    if (optionWidget) {
                        optionDialog->insertOptionPanel(name, optionWidget);
                        optionWidget->readConfig();
                        optionWidget->saveConfig();
                    }
                }
            }
        }
    });

    return true;
}

dpf::Plugin::ShutdownFlag OptionCore::stop()
{
    delete OptionDefaultKeeper::getOptionDialog();
    return Sync;
}
