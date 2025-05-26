// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"
#include "copilot.h"
#include "chatmanager.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"

#include <QMenu>

ChatReceiver::ChatReceiver(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<ChatReceiver>()
{
    using namespace std::placeholders;
    eventHandleMap.insert(editor.contextMenu.name, std::bind(&ChatReceiver::processContextMenuEvent, this, _1));
    eventHandleMap.insert(editor.selectionChanged.name, std::bind(&ChatReceiver::processSelectionChangedEvent, this, _1));
    eventHandleMap.insert(editor.inlineWidgetClosed.name, std::bind(&ChatReceiver::processInlineWidgetClosedEvent, this, _1));
    eventHandleMap.insert(notifyManager.actionInvoked.name, std::bind(&ChatReceiver::processActionInvokedEvent, this, _1));
    eventHandleMap.insert(project.openProject.name, std::bind(&ChatReceiver::processOpenProjectEvent, this, _1));
    eventHandleMap.insert(uiController.switchToWidget.name, std::bind(&ChatReceiver::processSwitchToWidget, this, _1));
    eventHandleMap.insert(ai.LLMChanged.name, std::bind(&ChatReceiver::processLLMChanged, this));
}

dpf::EventHandler::Type ChatReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ChatReceiver::topics()
{
    return { T_MENU, editor.topic, notifyManager.topic, project.topic, uiController.topic, ai.topic };
}

void ChatReceiver::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void ChatReceiver::processContextMenuEvent(const dpf::Event &event)
{
    QMenu *contextMenu = event.property("menu").value<QMenu *>();
    if (!contextMenu)
        return;

    contextMenu->addMenu(Copilot::instance()->getMenu());
}

void ChatReceiver::processSelectionChangedEvent(const dpf::Event &event)
{
    QString fileName = event.property("fileName").toString();
    int lineFrom = event.property("lineFrom").toInt();
    int indexFrom = event.property("indexFrom").toInt();
    int lineTo = event.property("lineTo").toInt();
    int indexTo = event.property("indexTo").toInt();
    Copilot::instance()->handleSelectionChanged(fileName, lineFrom, indexFrom, lineTo, indexTo);
    emit ChatCallProxy::instance()->selectionChanged();
}

void ChatReceiver::processInlineWidgetClosedEvent(const dpf::Event &event)
{
    Copilot::instance()->handleInlineWidgetClosed();
}

void ChatReceiver::processActionInvokedEvent(const dpf::Event &event)
{
    auto actId = event.property("actionId").toString();
    if (actId == "ai_rag_install")
        ChatManager::instance()->installConda();
}

void ChatReceiver::processOpenProjectEvent(const dpf::Event &event)
{
    QtConcurrent::run([=](){
        auto projectPath = event.property("workspace").toString();
        QJsonObject results = ChatManager::instance()->query(projectPath, "", 1);
        if (results["Chunks"].toArray().size() != 0)   // project has generated, update it
            ChatManager::instance()->generateRag(projectPath);
    });
}

void ChatReceiver::processSwitchToWidget(const dpf::Event &event)
{
    auto widgetName = event.property("name").toString();
    using namespace dpfservice;
    if (widgetName != MWNA_EDIT)
        return;
    auto windowService = dpfGetService(WindowService);
    windowService->showWidgetAtRightspace(MWNA_CHAT);
}

void ChatReceiver::processLLMChanged()
{
    emit ChatCallProxy::instance()->LLMsChanged();
}

ChatCallProxy::ChatCallProxy()
{
}

ChatCallProxy *ChatCallProxy::instance()
{
    static ChatCallProxy proxy;
    return &proxy;
}
