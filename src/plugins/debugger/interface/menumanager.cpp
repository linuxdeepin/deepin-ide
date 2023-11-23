// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menumanager.h"
#include "debugmanager.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "base/abstractmenu.h"
#include "common/common.h"
#include "services/window/windowservice.h"

#include <QMenu>

using namespace dpfservice;
MenuManager::MenuManager(QObject *parent) : QObject(parent)
{
}

void MenuManager::initialize(WindowService *windowService)
{
    if (!windowService)
        return;

    startDebugging.reset(new QAction(MWMDA_START_DEBUG));
    ActionManager::getInstance()->registerAction(startDebugging.get(), "Debug.Start.Debugging",
                                                 MWMDA_START_DEBUG, QKeySequence(Qt::Key::Key_F5),
                                                 ":/resource/images/debugger_start.svg");
    connect(startDebugging.get(), &QAction::triggered, debugManager, &DebugManager::run);
    AbstractAction *actionImpl = new AbstractAction(startDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Start Debugging", startDebugging.get(), "Debug");

#if 0 // not used yet.
    detachDebugger.reset(new QAction("Detach Debugger"));
    connect(detachDebugger.get(), &QAction::triggered, debugManager, &DebugManager::detachDebug);
    actionImpl = new AbstractAction(detachDebugger.get());
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);
#endif

    interrupt.reset(new QAction(MWMDA_INTERRUPT));
    ActionManager::getInstance()->registerAction(interrupt.get(), "Debug.Interrupt",
                                                 MWMDA_INTERRUPT, QKeySequence(Qt::Key::Key_F5),
                                                 ":/resource/images/debugger_interrupt.svg");
    interrupt->setEnabled(false);
    connect(interrupt.get(), &QAction::triggered, debugManager, &DebugManager::interruptDebug);
    actionImpl = new AbstractAction(interrupt.get());
    interrupt->setEnabled(false);
    windowService->addAction(MWM_DEBUG, actionImpl);

    continueDebugging.reset(new QAction(MWMDA_CONTINUE));
    ActionManager::getInstance()->registerAction(continueDebugging.get(), "Debug.Continue",
                                                 MWMDA_CONTINUE, QKeySequence(Qt::Key::Key_F5),
                                                 ":/resource/images/debugger_continue.svg");
    continueDebugging->setEnabled(false);
    connect(continueDebugging.get(), &QAction::triggered, debugManager, &DebugManager::continueDebug);
    actionImpl = new AbstractAction(continueDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Debugger.Continue", continueDebugging.get(), "Debug");

    abortDebugging.reset(new QAction(MWMDA_ABORT_DEBUGGING));
    ActionManager::getInstance()->registerAction(abortDebugging.get(), "Debug.Abort.Debugging",
                                                 MWMDA_ABORT_DEBUGGING, QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_Backspace),
                                                 ":/resource/images/debugger_stop.svg");
    abortDebugging->setEnabled(false);
    connect(abortDebugging.get(), &QAction::triggered, debugManager, &DebugManager::abortDebug);
    actionImpl = new AbstractAction(abortDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("abort_debug", abortDebugging.get(), "Debug");

    restartDebugging.reset(new QAction(MWMDA_RESTART_DEBUGGING));
    ActionManager::getInstance()->registerAction(restartDebugging.get(), "Debug.Restart.Debugging",
                                                 MWMDA_RESTART_DEBUGGING, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B),
                                                 ":/resource/images/restart_debug.svg");
    restartDebugging->setEnabled(false);
    connect(restartDebugging.get(), &QAction::triggered, debugManager, &DebugManager::restartDebug);
    actionImpl = new AbstractAction(restartDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);

    stepOver.reset(new QAction(MWMDA_STEP_OVER));
    ActionManager::getInstance()->registerAction(stepOver.get(), "Debug.Step.Over",
                                                 MWMDA_STEP_OVER, QKeySequence(Qt::Key::Key_F10),
                                                 ":/resource/images/debugger_stepover.svg");
    stepOver->setEnabled(false);
    connect(stepOver.get(), &QAction::triggered, debugManager, &DebugManager::stepOver);
    actionImpl = new AbstractAction(stepOver.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Step.Over", stepOver.get(), "Debug");

    stepIn.reset(new QAction(MWMDA_STEP_IN));
    ActionManager::getInstance()->registerAction(stepIn.get(), "Debug.Step.In",
                                                 MWMDA_STEP_IN, QKeySequence(Qt::Key::Key_F11),
                                                 ":/resource/images/debugger_stepinto.svg");
    stepIn->setEnabled(false);
    connect(stepIn.get(), &QAction::triggered, debugManager, &DebugManager::stepIn);
    actionImpl = new AbstractAction(stepIn.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Step.In", stepIn.get(), "Debug");

    stepOut.reset(new QAction(MWMDA_STEP_OUT));
    ActionManager::getInstance()->registerAction(stepOut.get(), "Debug.Step.Out",
                                                 MWMDA_STEP_OUT, QKeySequence(Qt::Modifier::SHIFT | Qt::Key::Key_F11),
                                                 ":/resource/images/debugger_stepout.svg");
    stepOut->setEnabled(false);
    connect(stepOut.get(), &QAction::triggered, debugManager, &DebugManager::stepOut);
    actionImpl = new AbstractAction(stepOut.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Step.Out", stepOut.get(), "Debug.End");
}

void MenuManager::handleRunStateChanged(AbstractDebugger::RunState state)
{
    switch (state) {
    case AbstractDebugger::kNoRun:
    case AbstractDebugger::kPreparing:
    case AbstractDebugger::kStart:
        startDebugging->setEnabled(true);
#if 0 // not used yet.
        detachDebugger->setEnabled(true);
#endif
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(false);
        restartDebugging->setEnabled(false);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        break;

    case AbstractDebugger::kRunning:
        startDebugging->setEnabled(false);
#if 0 // not used yet.
        detachDebugger->setEnabled(false);
#endif
        interrupt->setEnabled(true);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(true);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        break;
    case AbstractDebugger::kStopped:
        startDebugging->setEnabled(false);
#if 0 // not used yet.
        detachDebugger->setEnabled(false);
#endif
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(true);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(true);
        stepOver->setEnabled(true);
        stepIn->setEnabled(true);
        stepOut->setEnabled(true);
        break;
    case AbstractDebugger::kCustomRunning:
        startDebugging->setEnabled(false);
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(false);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        break;

    default:
        // do nothing.
        break;
    }
}
