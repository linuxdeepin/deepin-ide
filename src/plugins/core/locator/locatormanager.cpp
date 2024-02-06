// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "services/locator/locatorservice.h"
#include "locatormanager.h"

#include <QEvent>
#include <QKeyEvent>

using namespace dpfservice;
static LocatorManager *ins { nullptr };

AllLocators::AllLocators(QObject *parent)
    : abstractLocator(parent)
{
    setIncludedDefault(true);
}

void AllLocators::prepareSearch(const QString &searchText)
{
    Q_UNUSED(searchText);

    itemList.clear();
    QList<abstractLocator *> locators {};

    if (ins)
        locators = ins->getLocatorlist();

    foreach (auto locator, locators) {
        if (locator == this)
            continue;
        baseLocatorItem item(this);
        item.displayName = locator->getShortcut();
        item.extraInfo = locator->getDescription();
        item.id = locator->getDescription();
        item.icon = QIcon::fromTheme("go-next");

        itemList.append(item);
    }

    std::sort(itemList.begin(), itemList.end(), [=](const baseLocatorItem &itemA, const baseLocatorItem &itemB) {
        return itemA.displayName.toLower() < itemB.displayName.toLower();
    });
}

QList<baseLocatorItem> AllLocators::matchesFor(const QString &inputText)
{
    if (inputText.isEmpty())
        return itemList;
    return {};
}

void AllLocators::accept(baseLocatorItem item)
{
    ins->getInputEdit()->setText(item.displayName + QLatin1Char(' '));
}

LocatorManager::LocatorManager(QObject *parent)
    : QObject(parent)
{
    model = new locatorModel();
    inputEdit = new DSearchEdit();
    popupWidget = new PopupWidget(inputEdit);
    popupWidget->setWindowFlags(Qt::ToolTip);

    spinner = new DSpinner(inputEdit);
    spinner->setFixedSize(20, 20);
    spinner->hide();

    timer.setSingleShot(true);
    initConnect();

    popupWidget->setmodel(model);

    inputEdit->installEventFilter(this);
    installEventFilter(this);

    initService();
}

LocatorManager::~LocatorManager()
{
    if (model)
        delete model;
    if (inputEdit)
        delete inputEdit;
}

LocatorManager *LocatorManager::instance()
{
    if (!ins) {
        ins = new LocatorManager;
        auto allLocators = new AllLocators(ins);
        ins->registerLocator(allLocators);
    }
    return ins;
}

void LocatorManager::initConnect()
{
    connect(inputEdit, &DSearchEdit::textChanged, popupWidget, &PopupWidget::show);
    connect(inputEdit, &DSearchEdit::focusChanged, popupWidget, [=](bool on) {
        popupWidget->setVisible(on);
        updatePopupWidget(inputEdit->text());
    });
    connect(inputEdit, &DSearchEdit::textChanged, this, &LocatorManager::updatePopupWidget);
    connect(popupWidget, &PopupWidget::selectIndex, this, [=](const QModelIndex &index) {
        accept(index);
    });
    connect(this, &LocatorManager::handleKey, popupWidget, &PopupWidget::keyPressEvent);

    connect(&timer, &QTimer::timeout, this, &LocatorManager::showSpinner);
    connect(&watcher, &QFutureWatcher<bool>::finished, [&]() {
        popupWidget->selectFirstRow();
        timer.stop();
        if (searchAgain) {
            searchAgain = false;
            updatePopupWidget(inputEdit->text());
        } else if (spinner->isPlaying()) {
            spinner->hide();
            spinner->stop();
        }
    });
}

void LocatorManager::accept(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    auto row = index.row();
    if (row < 0 || row >= model->rowCount())
        return;

    auto item = model->getItems().at(row);
    item.locator->accept(item);
}

bool LocatorManager::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == inputEdit && event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Down:
        case Qt::Key_Tab:
        case Qt::Key_Up:
        case Qt::Key_Backtab:
            popupWidget->show();
            emit handleKey(keyEvent);
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            emit handleKey(keyEvent);
            return true;
        case Qt::Key_Escape:
            popupWidget->hide();
            return true;
        case Qt::Key_P:
        case Qt::Key_N:
            if (keyEvent->modifiers() == Qt::Modifier::CTRL) {
                popupWidget->show();
                emit handleKey(keyEvent);
                return true;
            }
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(watched, event);
}

void LocatorManager::registerLocator(abstractLocator *locator)
{
    if (!locator || locatorList.contains(locator))
        return;
    locatorList.append(locator);
}

QList<abstractLocator *> LocatorManager::getValidLocator(const QString &text, QString &searchText)
{
    const int length = text.size();
    int firstNonSpace;
    for (firstNonSpace = 0; firstNonSpace < length; ++firstNonSpace) {
        if (!text.at(firstNonSpace).isSpace())
            break;
    }
    const int whiteSpace = text.indexOf(QChar::Space, firstNonSpace);

    if (whiteSpace >= 0) {
        const QString prefix = text.mid(firstNonSpace, whiteSpace - firstNonSpace).toLower();
        QList<abstractLocator *> LocatorByShortCut;
        for (abstractLocator *locator : locatorList) {
            if (prefix == locator->getShortcut()) {
                searchText = text.mid(whiteSpace).trimmed();
                LocatorByShortCut << locator;
            }
        }

        if (!LocatorByShortCut.isEmpty())
            return LocatorByShortCut;
    }

    QList<abstractLocator *> LocatorByDefault;
    for (abstractLocator *locator : locatorList) {
        if (locator->isIncluedByDefault())
            LocatorByDefault << locator;
    }
    searchText = text.trimmed();
    return LocatorByDefault;
}

QList<baseLocatorItem> LocatorManager::runSearch(QList<abstractLocator *> validLocator, const QString &searchText)
{
    QList<baseLocatorItem> alreadyAdded {};
    foreach (auto locator, validLocator) {
        const QList<baseLocatorItem> matchResult = locator->matchesFor(searchText);
        alreadyAdded.reserve(matchResult.size());
        foreach (const baseLocatorItem &item, matchResult) {
            if (!alreadyAdded.contains(item))
                alreadyAdded.append(item);
        }
    }
    model->clear();
    model->addItems(alreadyAdded);
    return alreadyAdded;
}

void LocatorManager::updatePopupWidget(const QString &text)
{
    if (text.isEmpty())
        model->clear();

    //Not displayed spinner when processing time is less than 50ms
    timer.start(50);
    QString searchText;
    auto validLocator = getValidLocator(text, searchText);

    if (watcher.isRunning()) {
        searchAgain = true;
        return;
    }

    QFuture<void> future = QtConcurrent::run([=]() {
        foreach (auto locator, validLocator) {
            locator->prepareSearch(searchText);
        }
        runSearch(validLocator, searchText);
    });

    watcher.setFuture(future);
}

void LocatorManager::initService()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errstr;
    if (!ctx.load(dpfservice::LocatorService::name(), &errstr)) {
        qCritical() << errstr;
    }
    LocatorService *locatorService = ctx.service<LocatorService>(LocatorService::name());
    using namespace std::placeholders;
    if (!locatorService->registerLocator)
        locatorService->registerLocator = std::bind(&LocatorManager::registerLocator, this, _1);
}

void LocatorManager::showSpinner()
{
    spinner->move(inputEdit->width() - spinner->width() * 3, (inputEdit->height() - spinner->height()) / 2);
    spinner->start();
    spinner->show();
}
