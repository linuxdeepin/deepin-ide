// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTDISPLAYWIDGET_H
#define RECENTDISPLAYWIDGET_H

#include <DWidget>
#include "common/common.h"

class RecentDisplayWidgetPrivate;
class RecentDisplayWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
    RecentDisplayWidgetPrivate *const d;

public:
    explicit RecentDisplayWidget(DTK_WIDGET_NAMESPACE::DWidget *parent = nullptr);
    virtual ~RecentDisplayWidget() override;
    static RecentDisplayWidget *instance();

public slots:
    void addDocument(const QString &filePath);
    void addProject(const QString &kitName,
                    const QString &language,
                    const QString &workspace);
    void addSession(const QString &session);
    void removeSession(const QString &session);
    void updateSessions();

private slots:
    void doDoubleClicked(const QModelIndex &index);
    void btnOpenFileClicked();
    void btnOpenProjectClicked();
    void btnNewFileOrProClicked();
    void clearRecent();

private:
    void initializeUi();
    void initConnect();
    void initData();
    bool isProAndDocNull();
    QVariantMap parseProjectInfo(const QJsonObject &obj);

    void showEvent(QShowEvent *event) override;
};

#endif   // RECENTDISPLAYWIDGET_H
