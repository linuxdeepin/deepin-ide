// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGMODEL_H
#define DEBUGMODEL_H

#include "debug.h"
#include "debugsession.h"

#include <QObject>

namespace DEBUG_NAMESPACE {
class DebugModel : public QObject /*, public IDebugModel*/
{
    Q_OBJECT

public:
    DebugModel(dap::optional<dap::array<DebugSession *>> _sessions, QObject *parent = nullptr);
    dap::array<DebugSession *> getSessions(bool includeInactive = false);
    dap::optional<DebugSession *> getSession(dap::optional<dap::string> sessionId, bool includeInactive = false);
    void addSession(DebugSession *session);
    void clear();

    void rawUpdate(IRawModelUpdate *data);
    void clearThreads(dap::string id, bool removeThreads, dap::optional<dapNumber> reference);
    void fetchCallStack(Thread &thread);
    ReadonlyArray<IBreakpoint> getBreakpoints(dap::optional<QUrl> url, dap::optional<int> lineNumber,
                                              dap::optional<int> column, dap::optional<bool> enabledOnly);

    QMap<QString, dap::array<IBreakpoint>> getAllBreakpoints();
    ReadonlyArray<IFunctionBreakpoint> getFunctionBreakpoints();
    ReadonlyArray<IDataBreakpoint> getDataBreakpoints();
    ReadonlyArray<IExceptionBreakpoint> getExceptionBreakpoints();
    ReadonlyArray<IInstructionBreakpoint> getInstructionBreakpoints();

    bool areBreakpointsActivated();
    void setBreakpointsActivated(bool activated);
    ReadonlyArray<IBreakpoint> addBreakpoints(QUrl &uri, dap::array<IBreakpointData> &rawData, bool fireEvent = true);
    dap::array<IBreakpoint> removeBreakpoint(const QString &filePath, int lineNumber);
    dap::array<IBreakpoint> switchBreakpointStatus(const QString &filePath, int lineNumber, bool status);
    void updateBreakpoints(std::map<dap::string, IBreakpointUpdateData> &data);
    void setBreakpointSessionData(dap::string &sessionId, const dap::Capabilities &capabilites, dap::optional<std::map<dap::string, dap::Breakpoint>> data);
    dap::optional<dap::Breakpoint> getDebugProtocolBreakpoint(dap::string &breakpointId, dap::string &sessionId);
    void enableOrDisableAllBreakpoints(bool enable);
    IFunctionBreakpoint addFunctionBreakpoint(dap::string &functionName, dap::string &id);
    void updateFunctionBreakpoint(dap::string &id, dap::optional<dap::string> name, dap::optional<dap::string> hitCondition, dap::optional<dap::string> condition);
    void removeFunctionBreakpoints(dap::string &id);
    void addDataBreakpoint(dap::string &label, dap::string &dataId, bool canPersist, dap::optional<dap::array<dap::DataBreakpointAccessType>> accessTypes, dap::DataBreakpointAccessType accessType);
    void removeDataBreakpoints(dap::string &id);
#if 0
    void addInstructionBreakpoint(dap::string address, number offset, dap::optional<dap::string> condition, dap::optional<dap::string> hitCondition);
    void removeInstructionBreakpoints(dap::optional<dap::string> address);
    dap::array<Expression> getWatchExpressions();
    IExpression addWatchExpression(dap::optional<dap::string> name);
    void renameWatchExpression(dap::string &id, dap::string &newName);
    void removeWatchExpressions(dap::string &id);
    void moveWatchExpression(dap::string id, number position);
#endif
    void sourceIsNotAvailable(QUrl uri);
signals:

public slots:

private:
    bool breakpointsActivated = true;
    dap::array<Breakpoint> breakPoints;
    dap::array<IFunctionBreakpoint> functionBreakpoints;
    dap::array<IExceptionBreakpoint> exceptionBreakpoints;
    dap::array<IDataBreakpoint> dataBreakpoints;
    dap::array<IInstructionBreakpoint> instructionBreakpoints;
    dap::array<DebugSession *> sessions;
};

}   // end namespace.

#endif   // DEBUGMODEL_H
