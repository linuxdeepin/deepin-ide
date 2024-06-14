// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEEDITORRECEIVER_H
#define CODEEDITORRECEIVER_H

#include "common/common.h"
#include "framework.h"

class CodeEditorReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<CodeEditorReceiver>
{
    friend class dpf::AutoEventHandlerRegister<CodeEditorReceiver>;

public:
    explicit CodeEditorReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event &event) override;

private:
    void processOpenFileEvent(const dpf::Event &event);
    void processBackEvent(const dpf::Event &event);
    void processForwardEvent(const dpf::Event &event);
    void processGotoLineEvent(const dpf::Event &event);
    void processGotoPositionEvent(const dpf::Event &event);
    void processSetLineBackgroundColorEvent(const dpf::Event &event);
    void processResetLineBackgroundEvent(const dpf::Event &event);
    void processClearLineBackgroundEvent(const dpf::Event &event);
    void processSetModifiedAutoReloadEvent(const dpf::Event &event);

    // annotation
    void processAddAnnotationEvent(const dpf::Event &event);
    void processRemoveAnnotationEvent(const dpf::Event &event);
    void processClearAllAnnotationEvent(const dpf::Event &event);

    // debug
    void processAddBreakpointEvent(const dpf::Event &event);
    void processRemoveBreakpointEvent(const dpf::Event &event);
    void processSetBreakpointEnabledEvent(const dpf::Event &event);
    void processClearAllBreakpointsEvent(const dpf::Event &event);
    void processSetDebugLineEvent(const dpf::Event &event);
    void processRemoveDebugLineEvent(const dpf::Event &event);

private:
    QHash<QString, std::function<void(const dpf::Event &)>> eventHandleMap;
};

class EditorCallProxy : public QObject
{
    Q_OBJECT
    EditorCallProxy(const EditorCallProxy &) = delete;
    EditorCallProxy();

public:
    static EditorCallProxy *instance();

signals:
    void reqOpenFile(const QString &workspace, const QString &fileName);
    void reqBack();
    void reqForward();
    void reqGotoLine(const QString &fileName, int line);
    void reqGotoPosition(const QString &fileName, int line, int column);
    void reqSetLineBackgroundColor(const QString &fileName, int line, const QColor &color);
    void reqResetLineBackground(const QString &fileName, int line);
    void reqClearLineBackground(const QString &fileName);
    void reqSetModifiedAutoReload(const QString &fileName, bool flag);
    void reqDoRename(const newlsp::WorkspaceEdit &info);
    void reqCloseCurrentEditor();
    void reqSwitchHeaderSource();
    void reqFollowSymbolUnderCursor();
    void reqFindUsage();
    void reqRenameSymbol();

    // annotation
    void reqAddAnnotation(const QString &fileName, const QString &title, const QString &content,int line,  AnnotationType type);
    void reqRemoveAnnotation(const QString &fileName, const QString &title);
    void reqClearAllAnnotation(const QString &title);

    // debug
    void reqAddBreakpoint(const QString &fileName, int line, bool enabled);
    void reqRemoveBreakpoint(const QString &fileName, int line);
    void reqSetBreakpointEnabled(const QString &fileName, int line, bool enabled);
    void reqToggleBreakpoint();
    void reqClearAllBreakpoints();
    void reqSetDebugLine(const QString &fileName, int line);
    void reqRemoveDebugLine();
};

#endif   // CODEEDITORRECEIVER_H
