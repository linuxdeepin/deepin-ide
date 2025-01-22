// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLBAR_H
#define SYMBOLBAR_H

#include <QVariant>
#include <QScrollArea>
#include <QIcon>

class CurmbItem : public QWidget
{
    Q_OBJECT
public:
    enum CurmbType {
        FilePath,
        Symbol
    };

    explicit CurmbItem(CurmbType type, int index, QWidget *parent = nullptr);

    CurmbType curmbType() const;
    void setText(const QString &text);
    QString text() const;
    void setIcon(const QIcon &icon);
    void setSelected(bool selected);
    bool isSelected() const;
    bool isRoot() const;
    void setUserData(const QVariant &data);
    QVariant userData() const;

Q_SIGNALS:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEvent *event) override;
#else
    void enterEvent(QEnterEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QString displayText;
    QIcon itemIcon;
    int index { 0 };
    int spacing { 5 };
    bool hasSelected { false };
    bool isHover { false };
    QVariant data;
    CurmbType type;
};

class SymbolView;
class QHBoxLayout;
class SymbolBar : public QScrollArea
{
    Q_OBJECT
public:
    explicit SymbolBar(QWidget *parent = nullptr);

    void setPath(const QString &path);
    void clear();

public Q_SLOTS:
    void updateSymbol(int line, int index);
    void curmbItemClicked();
    void resetCurmbItemState();

private:
    CurmbItem *symbolItem { nullptr };
    SymbolView *symbolView { nullptr };
    QHBoxLayout *mainLayout { nullptr };
};

#endif   // SYMBOLBAR_H
