// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CVSKEEPER_H
#define CVSKEEPER_H

#include <QObject>

class SvnClientWidget;
class GitQlientWidget;
class CVSkeeper final: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CVSkeeper)
    explicit CVSkeeper(QObject *parent = nullptr);

public:
    static CVSkeeper *instance();
    void openRepos(const QString &repoPath);
    SvnClientWidget *svnMainWidget();
    GitQlientWidget *gitMainWidget();

private:
    SvnClientWidget *svnReposWidget;
    GitQlientWidget *gitReposWidget;
};

#endif // CVSKEEPER_H
