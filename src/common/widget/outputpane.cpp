// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "outputpane.h"
#include "common/common.h"

#include <DPlainTextEdit>
#include <DScrollBar>
#include <DMenu>

#include <QDebug>
#include <QVBoxLayout>

/**
 * @brief Output text color.
 */
const QColor kTextColorNormal(150, 150, 150);
const QColor kErrorMessageTextColor(255, 108, 108);
const QColor kMessageOutput(0, 135, 135);
constexpr int kDefaultMaxCharCount = 10000000;

DWIDGET_USE_NAMESPACE

class OutputPanePrivate
{
public:
    explicit OutputPanePrivate()
    {
    }

    ~OutputPanePrivate()
    {
    }

    bool enforceNewline = false;
    bool scrollToBottom = true;
    int maxCharCount = kDefaultMaxCharCount;
    QTextCursor cursor;
    DPlainTextEdit *outputEdit = nullptr;
    DMenu *menu = nullptr;
};

OutputPane::OutputPane(QWidget *parent)
    : QWidget(parent),
      d(new OutputPanePrivate())
{
    initUI();
}

OutputPane::~OutputPane()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void OutputPane::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    d->outputEdit = new DPlainTextEdit(this);
    d->outputEdit->setReadOnly(true);
    d->outputEdit->setLineWidth(0);

    d->cursor = QTextCursor(d->outputEdit->document());

    d->menu = new DMenu(this);
    d->menu->addActions(actionFactory());

    mainLayout->addWidget(d->outputEdit);
}

void OutputPane::clearContents()
{
    d->outputEdit->clear();
}

QString OutputPane::normalizeNewlines(const QString &text)
{
    QString res = text;
    res.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    return res;
}

bool OutputPane::isScrollbarAtBottom() const
{
    return d->outputEdit->verticalScrollBar()->value() == d->outputEdit->verticalScrollBar()->maximum();
}

QString OutputPane::doNewlineEnforcement(const QString &out)
{
    d->scrollToBottom = true;
    QString s = out;
    if (d->enforceNewline) {
        s.prepend(QLatin1Char('\n'));
        d->enforceNewline = false;
    }

    if (s.endsWith(QLatin1Char('\n'))) {
        d->enforceNewline = true;   // make appendOutputInline put in a newline next time
        s.chop(1);
    }

    return s;
}

void OutputPane::scrollToBottom()
{
    d->outputEdit->verticalScrollBar()->setValue(d->outputEdit->verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    d->outputEdit->verticalScrollBar()->setValue(d->outputEdit->verticalScrollBar()->maximum());
}

void OutputPane::appendCustomText(const QString &textIn, AppendMode mode, const QTextCharFormat &format)
{
    if (d->maxCharCount > 0 && d->outputEdit->document()->characterCount() >= d->maxCharCount) {
        qDebug() << "Maximum limit exceeded : " << d->maxCharCount;
        return;
    }
    if (!d->cursor.atEnd())
        d->cursor.movePosition(QTextCursor::End);

    if (mode == OverWrite) {
        d->cursor.select(QTextCursor::LineUnderCursor);
        d->cursor.removeSelectedText();
    }

    d->cursor.beginEditBlock();
    auto text = mode == OverWrite ? textIn.trimmed() : normalizeNewlines(doNewlineEnforcement(textIn));
    d->cursor.insertText(text, format);

    if (d->maxCharCount > 0 && d->outputEdit->document()->characterCount() >= d->maxCharCount) {
        QTextCharFormat tmp;
        tmp.setFontWeight(QFont::Bold);
        d->cursor.insertText(doNewlineEnforcement(tr("Additional output omitted") + QLatin1Char('\n')), tmp);
    }
    d->cursor.endEditBlock();

    scrollToBottom();
}

void OutputPane::appendText(const QString &text, OutputFormat format, AppendMode mode)
{
    QTextCharFormat textFormat;
    switch (format) {
    case OutputFormat::StdOut:
        textFormat.setForeground(kTextColorNormal);
        textFormat.setFontWeight(QFont::Normal);
        break;
    case OutputFormat::StdErr:
        textFormat.setForeground(kErrorMessageTextColor);
        textFormat.setFontWeight(QFont::Normal);
        break;
    case OutputFormat::NormalMessage:
        textFormat.setForeground(kMessageOutput);
        break;
    case OutputFormat::ErrorMessage:
        textFormat.setForeground(kErrorMessageTextColor);
        textFormat.setFontWeight(QFont::Bold);
        break;
    default:
        textFormat.setForeground(kTextColorNormal);
        textFormat.setFontWeight(QFont::Normal);
    }

    appendCustomText(text, mode, textFormat);
}

QTextDocument *OutputPane::document() const
{
    return d->outputEdit->document();
}

QPlainTextEdit *OutputPane::edit() const
{
    return d->outputEdit;
}

OutputPane *OutputPane::instance()
{
    static OutputPane *ins = new OutputPane();
    return ins;
}

void OutputPane::contextMenuEvent(QContextMenuEvent *event)
{
    if (!d->menu)
        return;

    d->menu->move(event->globalX(), event->globalY());
    d->menu->show();
}

QList<QAction *> OutputPane::actionFactory()
{
    QList<QAction *> list;

    {
        auto action = new QAction(this);
        action->setText(tr("Copy"));
        connect(action, &QAction::triggered, [this]() {
            if (!d->outputEdit->document()->toPlainText().isEmpty())
                d->outputEdit->copy();
        });
        list.append(action);
    }

    {
        auto action = new QAction(this);
        action->setText(tr("Clear"));
        connect(action, &QAction::triggered, [this]() {
            if (!d->outputEdit->document()->toPlainText().isEmpty())
                d->outputEdit->clear();
        });
        list.append(action);
    }

    {
        auto action = new QAction(this);
        action->setText(tr("Select All"));
        connect(action, &QAction::triggered, [this]() {
            if (!d->outputEdit->document()->toPlainText().isEmpty())
                d->outputEdit->selectAll();
        });
        list.append(action);
    }

    return list;
}
