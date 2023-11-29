// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKDELEGATE_H
#define TASKDELEGATE_H

#include "taskmodel.h"

#include <DStyledItemDelegate>

#include <QObject>
#include <QFont>

class TaskDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
    Q_OBJECT

    friend class TaskView; // for using Positions::minimumSize()

public:
    explicit TaskDelegate(QAbstractItemView *parent);
    ~TaskDelegate() override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // TaskView uses this method if the size of the taskview changes
    void emitSizeHintChanged(const QModelIndex &index);

    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void generateGradientPixmap(int width, int height, QColor color, bool selected) const;

    mutable int cachedHeight = 0;
    mutable QFont cachedFont;

    class Positions
    {
    public:
        Positions(const QStyleOptionViewItem &options, TaskModel *model) :
            totalWidth(options.rect.width()),
            maxFileLength(model->sizeOfFile(options.font)),
            maxLineLength(model->getSizeOfLineNumber(options.font)),
            realFileLength(maxFileLength),
            top(options.rect.top()),
            bottom(options.rect.bottom())
        {
            int flexibleArea = lineAreaLeft() - textAreaLeft() - ITEM_SPACING;
            if (maxFileLength > flexibleArea / 2)
                realFileLength = flexibleArea / 2;
            fontHeight = QFontMetrics(options.font).height();
        }

        int getTop() const { return top + ITEM_MARGIN; }
        int left() const { return ITEM_MARGIN; }
        int right() const { return totalWidth - ITEM_MARGIN; }
        int getBottom() const { return bottom; }
        int firstLineHeight() const { return fontHeight + 1; }
        static int minimumHeight() { return taskIconHeight() + 2 * ITEM_MARGIN; }

        int taskIconLeft() const { return left(); }
        static int taskIconWidth() { return TASK_ICON_SIZE; }
        static int taskIconHeight() { return TASK_ICON_SIZE; }
        int taskIconRight() const { return taskIconLeft() + taskIconWidth(); }
        QRect taskIcon() const { return QRect(taskIconLeft(), getTop(), taskIconWidth(), taskIconHeight()); }

        int textAreaLeft() const { return taskIconRight() + ITEM_SPACING; }
        int textAreaWidth() const { return textAreaRight() - textAreaLeft(); }
        int textAreaRight() const { return fileAreaLeft() - ITEM_SPACING; }
        QRect textArea() const { return QRect(textAreaLeft(), getTop(), textAreaWidth(), firstLineHeight()); }

        int fileAreaLeft() const { return fileAreaRight() - fileAreaWidth(); }
        int fileAreaWidth() const { return realFileLength; }
        int fileAreaRight() const { return lineAreaLeft() - ITEM_SPACING; }
        QRect fileArea() const { return QRect(fileAreaLeft(), getTop(), fileAreaWidth(), firstLineHeight()); }

        int lineAreaLeft() const { return lineAreaRight() - lineAreaWidth(); }
        int lineAreaWidth() const { return maxLineLength; }
        int lineAreaRight() const { return right(); }
        QRect lineArea() const { return QRect(lineAreaLeft(), getTop(), lineAreaWidth(), firstLineHeight()); }

    private:
        int totalWidth = 0;
        int maxFileLength = 0;
        int maxLineLength = 0;
        int realFileLength = 0;
        int top = 0;
        int bottom = 0;
        int fontHeight = 0;

        static const int TASK_ICON_SIZE = 16;
        static const int ITEM_MARGIN = 2;
        static const int ITEM_SPACING = 2 * ITEM_MARGIN;
    };
};

#endif // TASKDELEGATE_H
