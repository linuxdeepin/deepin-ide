// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexcompletionprovider.h"
#include "copilot.h"

using namespace CodeGeeX;

CodeGeeXCompletionProvider::CodeGeeXCompletionProvider(QObject *parent)
    : AbstractInlineCompletionProvider(parent)
{
    timer.setSingleShot(true);
    timer.setInterval(500);
}

QString CodeGeeXCompletionProvider::providerName() const
{
    return "CodeGeeX";
}

void CodeGeeXCompletionProvider::provideInlineCompletionItems(const Position &pos, const InlineCompletionContext &c)
{
    positon = pos;
    context = c;
    connect(&timer, &QTimer::timeout, Copilot::instance(), &Copilot::generateCode, Qt::UniqueConnection);
    timer.start();
}

QList<AbstractInlineCompletionProvider::InlineCompletionItem> CodeGeeXCompletionProvider::inlineCompletionItems() const
{
    return completionItems;
}

void CodeGeeXCompletionProvider::setInlineCompletionEnabled(bool enabled)
{
    if (!enabled && timer.isActive())
        timer.stop();

    completionEnabled = enabled;
}

void CodeGeeXCompletionProvider::setInlineCompletions(const QStringList &completions)
{
    completionItems.clear();
    for (const auto &completion : completions) {
        InlineCompletionItem item { completion, positon };
        completionItems << item;
    }
}

bool CodeGeeXCompletionProvider::inlineCompletionEnabled() const
{
    return completionEnabled;
}

AbstractInlineCompletionProvider::InlineCompletionContext CodeGeeXCompletionProvider::inlineCompletionContext() const
{
    return context;
}
