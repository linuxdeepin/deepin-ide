// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTDOCUMENTFIND_H
#define ABSTRACTDOCUMENTFIND_H

#include <QObject>

class AbstractDocumentFind : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDocumentFind(QObject *parent = nullptr);

    virtual QString findString() const = 0;

    virtual void findNext(const QString &txt) = 0;
    virtual void findPrevious(const QString &txt) = 0;
    virtual void replace(const QString &before, const QString &after);
    virtual void replaceFind(const QString &before, const QString &after);
    virtual void replaceAll(const QString &before, const QString &after);
    virtual void findStringChanged();
    virtual bool supportsReplace() const;
};

#endif   // ABSTRACTDOCUMENTFIND_H
