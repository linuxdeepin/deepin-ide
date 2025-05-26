// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "askpagewidget.h"
#include "intropage.h"
#include "messagecomponent.h"
#include "chatmanager.h"
#include "services/ai/aiservice.h"
#include "services/option/optionmanager.h"
#include "eventreceiver.h"

#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DScrollArea>
#include <DScrollBar>
#include <DHorizontalLine>
#include <DDialog>

#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QKeyEvent>
#include <QComboBox>

// button height + margin
static const int inputExtraHeight = 56;
static constexpr char* selectedLLM = "Selected_LLM";
// todo: provide a unified category after modifying the plugin name.
static constexpr char* optionCategory = "AskPageEdit";

AskPageWidget::AskPageWidget(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnection();
}

void AskPageWidget::setIntroPage()
{
    cleanWidgets();
    curState = Intro;
    IntroPage *introPage = new IntroPage(scrollArea);
    connect(introPage, &IntroPage::suggestionToSend, [this](const QString &suggesstion) {
        askQuestion(suggesstion);
    });
    scrollArea->setWidget(introPage);

    resetBtns();

    Q_EMIT introPageShown();
}

bool AskPageWidget::isIntroPageState()
{
    return curState == Intro;
}

void AskPageWidget::onMessageUpdate(const MessageData &msgData)
{
    if (curState == Intro) {
        curState = Session;
        setSessionPage();
    }

    if (!msgComponents.contains(msgData.messageID())) {
        if (waitingAnswer && msgData.messageType() == MessageData::Anwser) {
            msgComponents.insert(msgData.messageID(), waitComponets);
            waitingAnswer = false;
        } else {
            msgComponents.insert(msgData.messageID(), new MessageComponent(msgData, messageContainer));
            qobject_cast<QVBoxLayout *>(messageContainer->layout())->insertWidget(msgComponents.count() - 1, msgComponents.value(msgData.messageID()));
        }

        msgComponents.value(msgData.messageID())->updateMessage(msgData);
    } else {
        msgComponents.value(msgData.messageID())->updateMessage(msgData);
    }
}

void AskPageWidget::slotMessageSend()
{
    if (inputEdit) {
        auto prompt = inputEdit->edit()->toConvertedText();
        if (prompt.isEmpty())
            return;
        askQuestion(prompt);
    }
}

void AskPageWidget::onChatFinished()
{
    enterInputState();
}

void AskPageWidget::onDeleteBtnClicked()
{
    DDialog *confirmDialog = new DDialog(this);
    confirmDialog->setIcon(QIcon::fromTheme("dialog-warning"));
    confirmDialog->setMessage(tr("This operation will delete all the content of this session. confirm to delete it?"));
    confirmDialog->insertButton(0, tr("Cancel", "button"));
    confirmDialog->insertButton(1, tr("Delete", "button"), false, DDialog::ButtonWarning);

    connect(confirmDialog, &DDialog::buttonClicked, this, [=](int index) {
        if (index == 1) {
            ChatManager::instance()->deleteCurrentSession();
            setIntroPage();
        }
    });

    confirmDialog->exec();
}

void AskPageWidget::onHistoryBtnClicked()
{
    Q_EMIT requestShowHistoryPage();
}

void AskPageWidget::onCreateNewBtnClicked()
{
    //todo
}

void AskPageWidget::onLLMChanged(int index)
{
    auto llmInfo = LLMInfo::fromVariantMap(modelCb->itemData(index).toMap());
    if (!llmInfo.modelName.isEmpty())
        ChatManager::instance()->onLLMChanged(llmInfo);
    if (llmInfo.type != LLMType::ZHIPU_CODEGEEX) // <connect to network> function only Codegeex can use
        inputEdit->switchNetworkBtnVisible(false);
    else
        inputEdit->switchNetworkBtnVisible(true);

    onStopGenerate();

    OptionManager::getInstance()->setValue(optionCategory, selectedLLM, llmInfo.toVariant());
}

void AskPageWidget::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    scrollArea = new DScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignHCenter);
    layout->addWidget(scrollArea);

    //套一层Widget用以隐藏button和layout
    stopWidget = new DWidget(this);
    QHBoxLayout *hlayout = new QHBoxLayout;
    stopWidget->setLayout(hlayout);
    stopGenerate = new DPushButton(this);
    stopGenerate->setText(tr("stop generate"));
    hlayout->setContentsMargins(0, 20, 0, 20);
    hlayout->addWidget(stopGenerate);
    hlayout->setAlignment(Qt::AlignHCenter);
    layout->addWidget(stopWidget);
    stopWidget->hide();

    DHorizontalLine *line = new DHorizontalLine(this);
    layout->addWidget(line);

    inputWidget = new DWidget(this);
    layout->addWidget(inputWidget);

    initInputWidget();
}

void AskPageWidget::initInputWidget()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(10, 0, 10, 10);
    layout->setSpacing(0);
    inputWidget->setLayout(layout);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    layout->addLayout(btnLayout);
    btnLayout->setContentsMargins(0, 0, 0, 0);

    deleteBtn = new DToolButton(this);
    deleteBtn->setFixedSize(26, 26);
    deleteBtn->setIcon(QIcon::fromTheme("uc_chat_clear"));
    deleteBtn->setToolTip(tr("delete this session"));

    btnLayout->addWidget(deleteBtn);

    historyBtn = new DToolButton(this);
    historyBtn->setIcon(QIcon::fromTheme("uc_chat_history"));
    historyBtn->setFixedSize(26, 26);
    historyBtn->setToolTip(tr("history sessions"));
    historyBtn->hide(); // todo: Display after completion of functionality
    btnLayout->addWidget(historyBtn);

#if 0 // history not ready
    createNewBtn = new DToolButton(this);
    createNewBtn->setIcon(QIcon::fromTheme("uc_chat_new"));
    createNewBtn->setFixedSize(26, 26);
    createNewBtn->setToolTip(tr("create new session"));
    btnLayout->addWidget(createNewBtn);
#endif
    btnLayout->addStretch(1);

    modelCb = new QComboBox(this);
    modelCb->setFixedHeight(26);
    modelCb->setMaximumWidth(200);
    updateModelCb();
    btnLayout->addWidget(modelCb);

    inputEdit = new InputEditWidget(inputWidget);
    placeHolderText = tr("Ask question here, press Enter to send...");
    inputEdit->edit()->setPlaceholderText(placeHolderText);

    inputWidget->setFixedHeight(inputEdit->height() + inputExtraHeight);
    inputWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    layout->addWidget(inputEdit);
}

void AskPageWidget::initConnection()
{
    connect(ChatManager::instance(), &ChatManager::requestMessageUpdate, this, &AskPageWidget::onMessageUpdate);
    connect(ChatManager::instance(), &ChatManager::chatStarted, this, &AskPageWidget::enterAnswerState);
    connect(ChatManager::instance(), &ChatManager::chatFinished, this, &AskPageWidget::onChatFinished);
    connect(ChatManager::instance(), &ChatManager::setTextToSend, this, &AskPageWidget::setInputText);
    connect(ChatManager::instance(), &ChatManager::showCustomWidget, this, &AskPageWidget::showCustomWidget);
    connect(ChatManager::instance(), &ChatManager::terminated, this, &AskPageWidget::onStopGenerate);

    connect(inputEdit, &InputEditWidget::messageSended, this, &AskPageWidget::slotMessageSend);
    connect(inputEdit, &InputEditWidget::pressedEnter, this, &AskPageWidget::slotMessageSend);
    connect(deleteBtn, &DToolButton::clicked, this, &AskPageWidget::onDeleteBtnClicked);
    connect(historyBtn, &DToolButton::clicked, this, &AskPageWidget::onHistoryBtnClicked);
//    connect(createNewBtn, &DToolButton::clicked, this, &AskPageWidget::onCreateNewBtnClicked);
    connect(modelCb, qOverload<int>(&QComboBox::currentIndexChanged), this, &AskPageWidget::onLLMChanged);
    connect(ChatCallProxy::instance(), &ChatCallProxy::LLMsChanged, this, &AskPageWidget::updateModelCb);
    connect(inputEdit->edit(), &DTextEdit::textChanged, this, [this]() {
        inputWidget->setFixedHeight(inputEdit->height() + inputExtraHeight);
    });
    connect(stopGenerate, &DPushButton::clicked, this, &AskPageWidget::onStopGenerate);
    connect(scrollArea->verticalScrollBar(), &QScrollBar::rangeChanged, this, [=]() {
        if (scrollArea->verticalScrollBar()->isVisible()) {
            int maxValue = scrollArea->verticalScrollBar()->maximum();
            scrollArea->verticalScrollBar()->setValue(maxValue);
        }
    });
}

void AskPageWidget::cleanWidgets()
{
    if (auto currentWidget = scrollArea->takeWidget()) {
        currentWidget->deleteLater();
        msgComponents.clear();
    }
}

void AskPageWidget::setSessionPage()
{
    cleanWidgets();
    messageContainer = new DWidget(scrollArea);
    scrollArea->setWidget(messageContainer);

    QVBoxLayout *layout = new QVBoxLayout(messageContainer);
    layout->setContentsMargins(12, 0, 12, 20);
    layout->setSpacing(10);
    messageContainer->setLayout(layout);

    layout->addStretch(1);

    resetBtns();

    Q_EMIT sessionPageShown();
}

void AskPageWidget::enterAnswerState()
{
    if (curState == Intro) {
        curState = Session;
        setSessionPage();
    }

    progressCalcNum = 0;
    inputEdit->edit()->clear();
    inputEdit->disableSendBtn();
    inputEdit->setAnswering(true);

    if (deleteBtn)
        deleteBtn->setEnabled(false);
    if (historyBtn)
        historyBtn->setEnabled(false);
//    if (createNewBtn)
//        createNewBtn->setEnabled(false);

    stopWidget->show();
    waitForAnswer();
}

void AskPageWidget::enterInputState()
{
    stopWidget->hide();
    inputEdit->enableSendBtn();
    inputEdit->setAnswering(false);
    inputEdit->edit()->setPlaceholderText(placeHolderText);

    if (deleteBtn)
        deleteBtn->setEnabled(true);
    if (historyBtn)
        historyBtn->setEnabled(true);
//    if (createNewBtn)
//        createNewBtn->setEnabled(true);
}

void AskPageWidget::waitForAnswer()
{
    waitingAnswer = true;
    MessageData data("", MessageData::Anwser);
    waitComponets = new MessageComponent(data, messageContainer);
    qobject_cast<QVBoxLayout *>(messageContainer->layout())->insertWidget(msgComponents.count(), waitComponets);
    waitComponets->waitForAnswer();
}

void AskPageWidget::showCustomWidget(QWidget *widget)
{
    if (waitComponets) {
        if (!msgComponents.values().contains(waitComponets)) {
            QString stopId = "Stop:" + QString::number(QDateTime::currentMSecsSinceEpoch());
            msgComponents.insert(stopId, waitComponets);
        }
        waitComponets->stopWaiting();
    }
    waitComponets->setCustomWidget(widget);
}

void AskPageWidget::updateModelCb()
{
    using namespace dpfservice;
    auto aiSrv = dpfGetService(AiService);
    auto allLLMs = aiSrv->getAllModel();

    auto selectedLLMVariant = OptionManager::getInstance()->getValue(optionCategory, selectedLLM);
    auto userSelectedLLMInfo = LLMInfo::fromVariantMap(selectedLLMVariant.toMap());

    modelCb->clear();
    for (auto llm : allLLMs)
        modelCb->addItem(llm.icon, llm.modelName, llm.toVariant());

    if (!userSelectedLLMInfo.modelName.isEmpty())
        modelCb->setCurrentText(userSelectedLLMInfo.modelName);

    if (!modelCb->currentText().isEmpty())
        ChatManager::instance()->onLLMChanged(LLMInfo::fromVariantMap(modelCb->currentData().toMap()));
}

void AskPageWidget::onStopGenerate()
{
    if (!waitComponets)
        return;

    ChatManager::instance()->stopReceiving();
    if (!msgComponents.values().contains(waitComponets)) {
        QString stopId = "Stop:" + QString::number(QDateTime::currentMSecsSinceEpoch());
        msgComponents.insert(stopId, waitComponets);
    }
    waitComponets->stopWaiting();
    waitingAnswer = false;
    enterInputState();
}

void AskPageWidget::askQuestion(const QString &question)
{
    ChatManager::instance()->sendMessage(question);
}

void AskPageWidget::resetBtns()
{
    if (!deleteBtn || !historyBtn || !createNewBtn)
        return;

    deleteBtn->setEnabled(!isIntroPageState());
//    createNewBtn->setVisible(!isIntroPageState());
//    historyBtn->setVisible(true);
}

void AskPageWidget::setInputText(const QString &prompt)
{
    if (!waitingAnswer)
        inputEdit->edit()->setText(prompt);
}
