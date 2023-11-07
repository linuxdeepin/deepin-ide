// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITTABWIDGET_H
#define TEXTEDITTABWIDGET_H

#include "common/common.h"

#include <QWidget>

class TextEdit;
class TextEditTabWidgetPrivate;
class TextEditTabWidget : public QWidget
{
    Q_OBJECT
    friend class TextEditTabWidgetPrivate;
    TextEditTabWidgetPrivate *const d;
public:
    explicit TextEditTabWidget(QWidget *parent = nullptr);
    explicit TextEditTabWidget(TextEditTabWidget &text);
    virtual ~TextEditTabWidget();
    static TextEditTabWidget *instance();
    void setCloseButtonVisible(bool flag);
    void setSplitButtonVisible(bool flag);
    TextEdit *activeTextWidget();

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event = nullptr) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void closed();
    void splitClicked(Qt::Orientation, const newlsp::ProjectKey &, const QString &);
    void selected(bool state);
    void closeWidget();
    void sigOpenFile();

public slots:
    void openFile(const QString &filePath);
    void closeFile(const QString &filePath);
    void jumpToLine(const QString &filePath, int line);
    void jumpToRange(const QString &filePath, const newlsp::Range &range);
    void runningToLine(const QString &filePath, int line);
    void openFileWithKey(const newlsp::ProjectKey &key, const QString &filePath);
    void jumpToLineWithKey(const newlsp::ProjectKey &key, const QString &filePath, int line);
    void runningToLineWithKey(const newlsp::ProjectKey &key, const QString &filePath, int line);
    void runningEnd();
    void addDebugPoint(const QString &filePath, int line);
    void removeDebugPoint(const QString &filePath, int line);
    void debugPointClean();
    void replaceRange(const QString &filePath, const newlsp::Range &range,const QString &text);
    void setLineBackground(const QString &filePath, int line, const QColor &color);
    void delLineBackground(const QString &filePath, int line);
    void cleanLineBackground(const QString &filePath);
    void setAnnotation(const QString &filePath, int line, const QString &title, const AnnotationInfo &info);
    void cleanAnnotation(const QString &filePath, const QString &title);
    void cleanAllAnnotation(const QString &title);
    void selectSelf(bool state);
    void setModifiedAutoReload(const QString &filePath, bool flag);

private slots:
    void setDefaultFileEdit();
    void hideFileEdit(const QString &file);
    void showFileEdit(const QString &file);
    void hideFileStatusBar(const QString &file);
    void showFileStatusBar(const QString &file);
    void removeFileStatusBar(const QString &file);
    void removeFileEdit(const QString &file);
    void removeFileTab(const QString &file);
    void fileModifyed(const QString &file);
    void fileDeleted(const QString &file);
    void fileMoved(const QString &file);
    void doRenameReplace(const newlsp::WorkspaceEdit &renameResult);
    TextEdit *switchFileAndToOpen(const newlsp::ProjectKey &key, const QString &filePath);
    TextEdit *switchFileAndToOpen(const QString &filePath);
    void saveEditFile(const QString &file);

private:
    void handleDeletedFile(const QString &file);
    void detectFile(const QString &file);
};

#endif // TEXTEDITTABWIDGET_H
