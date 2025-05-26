// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputeditwidget.h"
#include "referencepopup.h"
#include "chatmanager.h"
#include "eventreceiver.h"
#include "services/editor/editorservice.h"
#include "services/window/windowservice.h"

#include <DTextEdit>
#include <DToolButton>
#include <DGuiApplicationHelper>

#include <QKeyEvent>
#include <QPainter>
#include <QStyleOptionFrame>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QAbstractTextDocumentLayout>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

static const int minInputEditHeight = 36;
static const int maxInputEditHeight = 236;

static const QString reference_opened_files = "OpendFiles";
static const QString reference_current_file = "CurrentFile";
static const QString reference_select_file = "SelectFile";
static const QString reference_codebase = "CodeBase";

TagTextFormat::TagTextFormat()
    : QTextCharFormat(QTextFormat(QTextFormat::InvalidFormat))
{
    setObjectType(QTextFormat::UserObject + 1);
}

void TagTextFormat::setText(const QString &text)
{
    setProperty(QTextFormat::UserProperty, text);
}

TagTextFormat::TagTextFormat(const QTextFormat &fmt)
    : QTextCharFormat(fmt)
{
}

class TagObjectInterface : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format) override
    {
        Q_UNUSED(doc);
        Q_UNUSED(posInDocument);
        const TagTextFormat tagFormat(format);
        const QFontMetricsF fm(tagFormat.font());
        return QSizeF(fm.horizontalAdvance(tagFormat.text()) + 5, fm.height());
    }

    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format) override
    {
        Q_UNUSED(doc);
        Q_UNUSED(posInDocument);
        const TagTextFormat tagFormat(format);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        using DTK_GUI_NAMESPACE::DPalette;

        DPalette dp(DTK_GUI_NAMESPACE::DGuiApplicationHelper::instance()->applicationPalette());

        auto color = dp.color(DPalette::LightLively);
        color.setAlpha(26);
        painter->setBrush(color);

        const QFontMetricsF fontMetrics(tagFormat.font());
        QRectF tagRect(rect.x(), rect.y(), rect.width(), fontMetrics.height() + 5);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(tagRect, 5, 5);
        QPen pen(dp.color(DPalette::LightLively));
        pen.setWidth(0);
        painter->setPen(pen);
        painter->drawText(tagRect, Qt::AlignCenter, tagFormat.text());
        painter->restore();
    }
};

QString TagTextFormat::text() const
{
    return property(QTextFormat::UserProperty).toString();
}

class InputEditWidgetPrivate
{
public:
    explicit InputEditWidgetPrivate(InputEditWidget *qq);
    InputEditWidget *q;
    InputEdit *edit { nullptr };
    DToolButton *sendButton { nullptr };
    DToolButton *netWorkBtn { nullptr };
    DToolButton *referenceBtn { nullptr };
    DToolButton *codeBaseBtn { nullptr };

    QWidget *buttonBox { nullptr };

    PopupWidget *referencePopup { nullptr };
    QList<ItemInfo> defaultReferenceItems;
    ItemModel model;

    QStringList selectedFiles;
    QMap<QString, QStringList> tagMap;

    bool isAnswering = false;

private:
    void initEdit();
    void initButtonBox();
    void initreferencePopup();
};

InputEditWidgetPrivate::InputEditWidgetPrivate(InputEditWidget *qq)
    : q(qq)
{
    initEdit();
    initButtonBox();
    initreferencePopup();
}

void InputEditWidgetPrivate::initEdit()
{
    edit = new InputEdit(q);
    edit->setAutoSelectCode(true);
    InputEditWidget::connect(edit, &InputEdit::textChanged, q, [this]() {
        auto currentText = edit->toPlainText();

        if (!isAnswering) {
            if (currentText.isEmpty())
                sendButton->setEnabled(false);
            else
                sendButton->setEnabled(true);
        }

        q->setFixedHeight(edit->height() + buttonBox->height());
        auto cursor = edit->textCursor();
        QString filterText = "";
        QString strBeforeCursor = "";
        while (cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor)) {
            strBeforeCursor = cursor.selectedText();
            if (strBeforeCursor.endsWith('@')) {
                q->popupReference();
            } else if (strBeforeCursor.contains(" ")) {
                referencePopup->hide();
                model.setFilterText("");
                return;
            } else if (strBeforeCursor.startsWith('@')) {
                filterText = strBeforeCursor;
                filterText.remove('@');
                break;
            }
        }

        if (!strBeforeCursor.contains('@')) {
            referencePopup->hide();
            return;
        }

        model.setFilterText(filterText);
        if (model.getItems().isEmpty())
            referencePopup->hide();
        else
            referencePopup->show();
    });
}

void InputEditWidgetPrivate::initButtonBox()
{
    buttonBox = new QWidget(q);
    buttonBox->setFixedHeight(minInputEditHeight);
    auto hLayout = new QHBoxLayout(buttonBox);
    hLayout->setContentsMargins(6, 6, 6, 6);
    hLayout->setAlignment(Qt::AlignRight);
    hLayout->setSpacing(0);

    sendButton = new DToolButton(q);
    sendButton->setFixedSize(24, 24);
    sendButton->setIcon(QIcon::fromTheme("uc_chat_send"));
    sendButton->setEnabled(false);

    codeBaseBtn = new DToolButton(q);
    codeBaseBtn->setFixedSize(24, 24);
    codeBaseBtn->setIcon(QIcon::fromTheme("uc_chat_project"));
    codeBaseBtn->setToolTip(InputEditWidget::tr("reference codebase"));
    codeBaseBtn->setCheckable(true);
#ifndef SUPPORTMINIFORGE
    codeBaseBtn->hide();
#endif

    referenceBtn = new DToolButton(q);
    referenceBtn->setFixedSize(24, 24);
    referenceBtn->setIcon(QIcon::fromTheme("uc_chat_files"));
    referenceBtn->setToolTip(InputEditWidget::tr("reference"));

    netWorkBtn = new DToolButton(q);
    netWorkBtn->setFixedSize(24, 24);
    netWorkBtn->setCheckable(true);
    netWorkBtn->setIcon(QIcon::fromTheme("uc_chat_internet"));
    netWorkBtn->setToolTip(InputEditWidget::tr("connect to network"));

    InputEditWidget::connect(sendButton, &DToolButton::clicked, q, &InputEditWidget::messageSended);
    InputEditWidget::connect(codeBaseBtn, &DToolButton::clicked, q, &InputEditWidget::onCodeBaseBtnClicked);
    InputEditWidget::connect(referenceBtn, &DToolButton::clicked, q, &InputEditWidget::onReferenceBtnClicked);
    InputEditWidget::connect(netWorkBtn, &DToolButton::clicked, q, &InputEditWidget::onNetWorkBtnClicked);
    hLayout->addWidget(codeBaseBtn);
    hLayout->addWidget(referenceBtn);
    hLayout->addWidget(netWorkBtn);
    hLayout->addWidget(sendButton);
}

void InputEditWidgetPrivate::initreferencePopup()
{
    referencePopup = new PopupWidget(q);
    referencePopup->setWindowFlags(Qt::ToolTip);
    referencePopup->setmodel(&model);

    ItemInfo currentFile;
    currentFile.type = reference_current_file;
    currentFile.displayName = InputEditWidget::tr("Current File");
    ItemInfo selectFile;
    selectFile.type = reference_select_file;
    selectFile.displayName = InputEditWidget::tr("Select File");
    ItemInfo openedFiles;
    openedFiles.type = reference_opened_files;
    openedFiles.displayName = InputEditWidget::tr("Opened Files");
#ifdef SUPPORTMINIFORGE
        ItemInfo codeBase;
        codeBase.type = reference_codebase;
        codeBase.displayName = InputEditWidget::tr("CodeBase");
        defaultReferenceItems = QList { currentFile, selectFile, openedFiles, codeBase };
#else
        defaultReferenceItems = QList { currentFile, selectFile, openedFiles };
#endif
}

InputEdit::InputEdit(QWidget *parent)
    : DTextEdit(parent)
{
    setMinimumHeight(minInputEditHeight);
    setFixedHeight(minInputEditHeight);
    setLineWrapMode(QTextEdit::WidgetWidth);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptRichText(false);
    document()->documentLayout()->registerHandler(QTextFormat::UserObject + 1, new TagObjectInterface);

    connect(this, &DTextEdit::textChanged, this, &InputEdit::onTextChanged);
    connect(ChatCallProxy::instance(), &ChatCallProxy::selectionChanged, this, [=](){
        if (!autoSelectCode)
            return;
        auto editorService = dpfGetService(EditorService);
        QString currentFile = editorService->currentFile();
        QString selectedCode = editorService->getSelectedText();
        if (currentFile.isEmpty() || selectedCode.isEmpty()) {
            if (!selectedCodeTag.isEmpty())
                removeTag(selectedCodeTag);
            this->selectedCode.clear();
            this->selectedCodeTag.clear();
            return;
        }

        Edit::Range selectedRange = editorService->selectionRange(currentFile);
        QString tagText = QString("%1:L%2C%3-L%4C%5").arg(QFileInfo(currentFile).fileName(),
                                                          QString::number(selectedRange.start.line + 1),
                                                          QString::number(selectedRange.start.column + 1),
                                                          QString::number(selectedRange.end.line + 1),
                                                          QString::number(selectedRange.end.column + 1));

        removeTag(selectedCodeTag);
        appendTag(tagText);
        this->selectedCode = selectedCode;
        this->selectedCodeTag = tagText;
    });
}

void InputEdit::onTextChanged()
{
    auto adjustHeight = document()->size().height();
    if (adjustHeight < minInputEditHeight)
        setFixedHeight(minInputEditHeight);
    else if (adjustHeight > maxInputEditHeight)
        setFixedHeight(maxInputEditHeight);
    else
        setFixedHeight(adjustHeight);

    QTextCursor cursor(document());
    QSet<QString> tagList;
    int last_pos = 0;

    // bug: tag will removed when send message. and causes the tag to be reset before it is used.
    // update tag when llm is not running
    if (ChatManager::instance()->checkRunningState(true))
        return;

    cursor.setPosition(0);
    formatList.clear();

    while (!cursor.atEnd()) {
        cursor.setPosition(last_pos + 1);

        if (last_pos == cursor.position())
            break;
        last_pos = cursor.position();

        TagTextFormat format(cursor.charFormat());
        if (format.objectType() == QTextFormat::UserObject + 1) {
            const QString &text = format.text();

            if (!text.isEmpty()) {
                tagList << text;
                formatList << text;

                if (!formats.contains(text)) {
                    formats[text] = format;
                    Q_EMIT tagAdded(text);
                }
            }
        }
    }

    Q_FOREACH (const TagTextFormat &f, formats) {
        if (!tagList.contains(f.text())) {
            formats.remove(f.text());
            Q_EMIT tagRemoved(f.text());
        }
    }
}

bool InputEdit::event(QEvent *e)
{
    if (e->type() == QEvent::Show)
        setFocus();

    return DTextEdit::event(e);
}

void InputEdit::focusOutEvent(QFocusEvent *e)
{
    DTextEdit::focusOutEvent(e);
    emit focusOut();
}

QString InputEdit::toConvertedText() const
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);

    QString text;
    while (!cursor.atEnd()) {
        auto format = cursor.charFormat();
        auto currentText = cursor.selectedText();
        text.append(cursor.selectedText());

        if (format.type() == QTextFormat::InvalidFormat && (currentText == QString(QChar::ObjectReplacementCharacter))) {
            auto innerText = format.property(QTextFormat::UserProperty).toString();
            if (innerText.mid(1) == selectedCodeTag) {
                text.append("\n```\n" + innerText + "\n" + selectedCode + "\n```\n");
            } else if (!innerText.isEmpty()) {
                text.append('`' + innerText + '`');
            }
        }

        cursor.clearSelection();
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }

    text.append(cursor.selectedText());
    return text.remove(QChar::ObjectReplacementCharacter);
}

void InputEdit::appendTag(const QString &text)
{
    QTextCursor cursor = textCursor();
    QTextCharFormat originalFormat = cursor.charFormat();

    if (toPlainText().contains('@')) {
        while (cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor)) {
            if (cursor.selectedText().at(0) == "@")
                break;
        }
    }

    auto selectedText = cursor.selectedText();
    if (selectedText.startsWith('@'))
        cursor.removeSelectedText();

    QString tagText = text;
    if (!text.startsWith('@'))
        tagText.prepend('@');
    TagTextFormat tagFormat;
    tagFormat.setText(tagText);
    formats.insert(tagText, tagFormat);

    cursor.insertText(QString(QChar::ObjectReplacementCharacter), originalFormat);
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), tagFormat);
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), originalFormat);
}

void InputEdit::removeTag(const QString &tag)
{
    if (!hasTag(tag))
        return;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    auto target = '@' + tag;
    while (!cursor.atEnd()) {
        auto format = cursor.charFormat();
        if (format.type() == QTextFormat::InvalidFormat) { // tagTextFormat set it to
            if (target == format.property(QTextFormat::UserProperty).toString()) {
                cursor.removeSelectedText();
                formats.remove(target);
                cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
                cursor.removeSelectedText(); // remove original format at left
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                format = cursor.charFormat();
                // remove original format and abnormal text at Right
                while (format.type() == QTextFormat::InvalidFormat && (target == format.property(QTextFormat::UserProperty).toString())) {
                    format = cursor.charFormat();
                    cursor.removeSelectedText();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    format = cursor.charFormat();
                }
                cursor.removeSelectedText();
            }
        }
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
}

bool InputEdit::hasTag(const QString &text)
{
    if (text.startsWith('@'))
        return formats.contains(text);
    else
        return formats.contains('@' + text);
}

void InputEditWidget::onReferenceBtnClicked()
{
    auto cursor = d->edit->textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    auto charBeforeCursor = cursor.selectedText();
    if (charBeforeCursor != "@")
        cursor.insertText("@");
    else if (!d->referencePopup->isVisible())
        d->referencePopup->show();
    d->edit->setFocus();
}

void InputEditWidget::onCodeBaseBtnClicked()
{
    bool checked = d->codeBaseBtn->isChecked();
    if (!checked && d->edit->hasTag(reference_codebase)) // cancel the button , but @codebase in edit
        return;
    ChatManager::instance()->setReferenceCodebase(checked);
}

void InputEditWidget::onNetWorkBtnClicked()
{
    ChatManager::instance()->connectToNetWork(d->netWorkBtn->isChecked());
}

InputEditWidget::InputEditWidget(QWidget *parent)
    : DFrame(parent), d(new InputEditWidgetPrivate(this))
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(d->edit);
    mainLayout->addWidget(d->buttonBox);
    installEventFilter(this);
    d->edit->installEventFilter(this);

    setFixedHeight(d->edit->height() + d->buttonBox->height());
    connect(this, &InputEditWidget::handleKey, d->referencePopup, &PopupWidget::keyPressEvent);
    connect(d->edit, &InputEdit::enterReference, this, &InputEditWidget::popupReference);
    connect(d->referencePopup, &PopupWidget::selectIndex, this, &InputEditWidget::accept);
    connect(d->edit, &InputEdit::focusOut, d->referencePopup, &PopupWidget::hide);
    connect(d->edit, &InputEdit::tagAdded, this, &InputEditWidget::onTagAdded);
    connect(d->edit, &InputEdit::tagRemoved, this, &InputEditWidget::onTagRemoved);
}

bool InputEditWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing);

        QStyleOptionFrame panel;
        initStyleOption(&panel);
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);

        return true;
    }

    return DFrame::event(e);
}

bool InputEditWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->edit) {
        if (event->type() == QEvent::Paint) {
            return true;   // do not show border when get focus
        } else if (event->type() == QEvent::KeyPress) {
            auto keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Down:
            case Qt::Key_Tab:
            case Qt::Key_Up:
            case Qt::Key_Backtab:
                emit handleKey(keyEvent);
                return true;
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (keyEvent->modifiers() & Qt::AltModifier)
                    d->edit->insertPlainText("\n");
                else if (!d->referencePopup->isVisible() && !d->isAnswering)
                    emit pressedEnter();
                else
                    emit handleKey(keyEvent);
                return true;
            case Qt::Key_Space:
            case Qt::Key_Escape:
                d->referencePopup->hide();
                break;
            case Qt::Key_Left: {
                auto cursor = d->edit->textCursor();
                cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
                auto charSelected = cursor.selectedText();
                if (charSelected == '@')
                    d->referencePopup->hide();
            } break;
            default:
                break;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

InputEdit *InputEditWidget::edit()
{
    return d->edit;
}

void InputEditWidget::popupReference()
{
    d->model.clear();
    d->model.addItems(d->defaultReferenceItems);
    d->referencePopup->show();
    d->referencePopup->selectFirstRow();
}

void InputEditWidget::accept(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    auto row = index.row();
    if (row < 0 || row >= d->model.rowCount())
        return;

    EditorService *editorSrv = dpfGetService(EditorService);
    ItemInfo item = d->model.getItems().at(row);

    auto notify = [=](const QString &message){
        WindowService *windowSrv = dpfGetService(WindowService);
        windowSrv->notify(2, "Chat", message, {});
    };

    auto appendTag = [=](const QString &filePath) {
        QFileInfo info(filePath);
        d->selectedFiles.append(filePath);
        auto tag = "file: " + info.dir().dirName() + '/' + info.fileName();
        d->edit->appendTag(tag);
        d->tagMap.insert('@' + tag, { filePath });
    };
    if (item.type == reference_current_file) {
        auto filePath = editorSrv->currentFile();
        if (filePath.isEmpty()) {
            notify(tr("No opened file"));
            return;
        }
        appendTag(filePath);
    } else if (item.type == reference_select_file) {
        QString result = QFileDialog::getOpenFileName(this, QAction::tr("Select File"), QDir::homePath());
        if (result.isEmpty())
            return;
        appendTag(result);
    } else if (item.type == reference_opened_files) {
        auto openedFiles = editorSrv->openedFiles();
        if (openedFiles.isEmpty()) {
            notify(tr("No opened file"));
            return;
        }
        QList<ItemInfo> items;
        for (auto file : openedFiles) {
            ItemInfo item;
            item.extraInfo = file;
            item.displayName = QFileInfo(file).fileName();
            items.append(item);
        }
        d->model.clear();
        d->model.addItems(items);
        return;
    } else if (item.type == reference_codebase) {
        ChatManager::instance()->setReferenceCodebase(true);
        d->edit->appendTag(reference_codebase);
    } else if (!item.extraInfo.isEmpty()) {
        appendTag(item.extraInfo);
    }

    d->referencePopup->hide();
    ChatManager::instance()->setReferenceFiles(d->selectedFiles);
}

void InputEditWidget::switchNetworkBtnVisible(bool visible)
{
    d->netWorkBtn->setVisible(visible);
    if (!visible) {
        d->netWorkBtn->setChecked(false);
        ChatManager::instance()->connectToNetWork(false);
    }
}

void InputEditWidget::enableSendBtn()
{
    d->sendButton->setEnabled(true);
}

void InputEditWidget::disableSendBtn()
{
    d->sendButton->setEnabled(false);
}

void InputEditWidget::setAnswering(bool isAnswering)
{
    d->isAnswering = isAnswering;
}

// use to restore tag, : remove tag then Ctrl+z
void InputEditWidget::onTagAdded(const QString &text)
{
    if (text.mid(1) == reference_codebase)
        ChatManager::instance()->setReferenceCodebase(true);
    if (!d->tagMap.contains(text))
        return;
    d->selectedFiles.append(d->tagMap[text]);
    ChatManager::instance()->setReferenceFiles(d->selectedFiles);
}

void InputEditWidget::onTagRemoved(const QString &text)
{
    if (text.mid(1) == reference_codebase && !d->codeBaseBtn->isChecked()) //remove @
        ChatManager::instance()->setReferenceCodebase(false);
    if (!d->tagMap.contains(text))
        return;
    for (auto item : d->tagMap[text])
        d->selectedFiles.removeAll(item);

    ChatManager::instance()->setReferenceFiles(d->selectedFiles);
}

#include "inputeditwidget.moc"
