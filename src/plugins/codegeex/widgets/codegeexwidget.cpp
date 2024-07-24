// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexwidget.h"
#include "askpagewidget.h"
#include "historylistwidget.h"
#include "translationpagewidget.h"
#include "codegeexmanager.h"
#include "copilot.h"

#include <DLabel>
#include <DStackedWidget>
#include <DSuggestButton>

#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QResizeEvent>

CodeGeeXWidget::CodeGeeXWidget(QWidget *parent)
    : DFrame(parent)
{
    initUI();
    initConnection();
}

void CodeGeeXWidget::onLoginSuccessed()
{
    auto mainLayout = qobject_cast<QVBoxLayout *>(layout());
    if (mainLayout) {
        QLayoutItem *item = nullptr;
        while ((item = mainLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }

    initAskWidget();
    initHistoryWidget();
    CodeGeeXManager::instance()->createNewSession();
}

void CodeGeeXWidget::onLogOut()
{
    auto mainLayout = qobject_cast<QVBoxLayout *>(layout());
    if (mainLayout) {
        QLayoutItem *item = nullptr;
        while ((item = mainLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }

    delete mainLayout;
    initUI();
}

void CodeGeeXWidget::onNewSessionCreated()
{
    stackWidget->setCurrentIndex(1);

    if (askPage)
        askPage->setIntroPage();
}

void CodeGeeXWidget::toTranslateCode(const QString &code)
{
    switchPage(pageState::TrasnlatePage);
    transPage->setInputEditText(code);
    transPage->cleanOutputEdit();
}

void CodeGeeXWidget::switchPage(pageState state)
{
    if (state == pageState::TrasnlatePage) {
        tabBar->buttonList().at(0)->setChecked(false);
        tabBar->buttonList().at(1)->setChecked(true);
        stackWidget->setCurrentWidget(transPage);
    } else {
        tabBar->buttonList().at(0)->setChecked(true);
        tabBar->buttonList().at(1)->setChecked(false);
        stackWidget->setCurrentWidget(askPage);
    }
}

void CodeGeeXWidget::onCloseHistoryWidget()
{
    historyWidgetAnimation->setStartValue(QRect(0, 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->setEndValue(QRect(-this->rect().width(), 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->start();

    historyShowed = false;
}

void CodeGeeXWidget::onShowHistoryWidget()
{
    CodeGeeXManager::instance()->fetchSessionRecords();

    if (!historyWidget || !historyWidgetAnimation)
        return;

    historyWidgetAnimation->setStartValue(QRect(-this->rect().width(), 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->setEndValue(QRect(0, 0, historyWidget->width(), historyWidget->height()));
    historyWidgetAnimation->start();

    historyShowed = true;
}

void CodeGeeXWidget::resizeEvent(QResizeEvent *event)
{
    if (historyWidget) {
        if (historyShowed) {
            historyWidget->setGeometry(0, 0, this->width(), this->height());
        } else {
            historyWidget->setGeometry(-this->width(), 0, this->width(), this->height());
        }
    }

    DWidget::resizeEvent(event);
}

void CodeGeeXWidget::initUI()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContentsMargins(0, 0, 0, 0);

    auto initLoginUI = [this]() {
        auto mainLayout = new QVBoxLayout(this);
        auto loginWidget = new DWidget(this);
        loginWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        auto verticalLayout = new QVBoxLayout(loginWidget);
        verticalLayout->setAlignment(Qt::AlignCenter);
        verticalLayout->setContentsMargins(50, 0, 50, 50);

        auto label_icon = new DLabel(this);
        label_icon->setPixmap(QIcon::fromTheme("codegeex_logo").pixmap(QSize(40, 26)));
        label_icon->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_icon);

        auto welcome_label = new DLabel(loginWidget);
        welcome_label->setText(tr("Welcome to CodeGeeX"));//\nA must-have all-round AI tool for developers
        welcome_label->setAlignment(Qt::AlignCenter);

        auto font = welcome_label->font();
        font.setPixelSize(14);
        font.setWeight(500);
        welcome_label->setFont(font);

        auto descrption_label = new DLabel(loginWidget);
        descrption_label->setText(tr("A must-have all-round AI tool for developers"));
        descrption_label->setAlignment(Qt::AlignCenter);

        font = descrption_label->font();
        font.setPixelSize(12);
        font.setWeight(400);
        descrption_label->setFont(font);

        verticalLayout->addSpacing(30);
        verticalLayout->addWidget(welcome_label);
        verticalLayout->addSpacing(5);
        verticalLayout->addWidget(descrption_label);

        auto btnLayout = new QHBoxLayout;     //make DSuggestBtn alignCenter
        auto loginBtn = new DSuggestButton(loginWidget);
        loginBtn->setFixedSize(200, 36);
        loginBtn->setText(tr("Go to login"));
        connect(loginBtn, &DSuggestButton::clicked, this, [=] {
            CodeGeeXManager::instance()->login();
        });

        btnLayout->addWidget(loginBtn, Qt::AlignHCenter);

        verticalLayout->addSpacing(30);
        verticalLayout->addLayout(btnLayout, Qt::AlignCenter);

        mainLayout->addWidget(loginWidget);
    };
    initLoginUI();
}

void CodeGeeXWidget::initConnection()
{
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::loginSuccessed, this, &CodeGeeXWidget::onLoginSuccessed);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::logoutSuccessed, this, &CodeGeeXWidget::onLogOut);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::createdNewSession, this, &CodeGeeXWidget::onNewSessionCreated);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::requestToTransCode, this, &CodeGeeXWidget::toTranslateCode);
    connect(CodeGeeXManager::instance(), &CodeGeeXManager::requestMessageUpdate, this, [=](){switchPage(pageState::AskPage);});
}

void CodeGeeXWidget::initAskWidget()
{
    QHBoxLayout *tabLayout = new QHBoxLayout;
    tabLayout->setContentsMargins(0, 20, 0, 20);
    tabLayout->setAlignment(Qt::AlignHCenter);

    //套一层DWidget，用以登出时删除现有窗口。 直接使用layout会删不掉。
    DWidget *tabWidget = new DWidget(this);
    tabWidget->setLayout(tabLayout);

    tabBar = new DButtonBox(this);
    tabLayout->addWidget(tabBar, 0);

    stackWidget = new QStackedWidget(this);
    stackWidget->setContentsMargins(0, 0, 0, 0);
    stackWidget->setFrameShape(QFrame::NoFrame);
    stackWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

    auto mainLayout = qobject_cast<QVBoxLayout *>(layout());
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(tabWidget, 0);
    mainLayout->addWidget(stackWidget, 1);

    initTabBar();
    initStackWidget();
    initAskWidgetConnection();
}

void CodeGeeXWidget::initHistoryWidget()
{
    historyWidget = new HistoryListWidget(this);
    historyWidget->setGeometry(-this->width(), 0, this->width(), this->height());
    historyWidget->show();

    historyWidgetAnimation = new QPropertyAnimation(historyWidget, "geometry");
    historyWidgetAnimation->setEasingCurve(QEasingCurve::InOutSine);
    historyWidgetAnimation->setDuration(300);

    initHistoryWidgetConnection();
}

void CodeGeeXWidget::initTabBar()
{
    DButtonBoxButton *askbtn = new DButtonBoxButton(tr("Ask CodeGeeX"), this);
    askbtn->setCheckable(true);
    askbtn->setChecked(true);
    DButtonBoxButton *trsbtn = new DButtonBoxButton(tr("Translation"), this);

    tabBar->setButtonList({ askbtn, trsbtn }, true);
}

void CodeGeeXWidget::initStackWidget()
{
    askPage = new AskPageWidget(this);
    transPage = new TranslationPageWidget(this);

    DWidget *creatingSessionWidget = new DWidget(this);
    QHBoxLayout *layout = new QHBoxLayout;
    creatingSessionWidget->setLayout(layout);

    DLabel *creatingLabel = new DLabel(creatingSessionWidget);
    creatingLabel->setAlignment(Qt::AlignCenter);
    creatingLabel->setText(tr("Creating a new session..."));
    layout->addWidget(creatingLabel);

    stackWidget->insertWidget(0, creatingSessionWidget);
    stackWidget->insertWidget(1, askPage);
    stackWidget->insertWidget(2, transPage);
    stackWidget->setCurrentIndex(0);
}

void CodeGeeXWidget::initAskWidgetConnection()
{
    connect(tabBar, &DButtonBox::buttonClicked, stackWidget, [=](QAbstractButton *button) {
        if (button->text() == tr("Ask CodeGeeX"))
            stackWidget->setCurrentWidget(askPage);
        else
            stackWidget->setCurrentWidget(transPage);
    });
    connect(askPage, &AskPageWidget::requestShowHistoryPage, this, &CodeGeeXWidget::onShowHistoryWidget);
}

void CodeGeeXWidget::initHistoryWidgetConnection()
{
    connect(historyWidget, &HistoryListWidget::requestCloseHistoryWidget, this, &CodeGeeXWidget::onCloseHistoryWidget);
}
