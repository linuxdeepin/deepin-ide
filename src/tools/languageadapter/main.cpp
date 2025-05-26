// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/common.h"
#include "jsonrpccallproxy.h"
#include "remotechecker.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonObject>
#include <QDir>

#include <iostream>

static QString packageInstallPath(const QString &python)
{
    auto getVersion = [](const QString &output) -> QString {
        static QRegularExpression regex(R"((\d{1,3}(?:\.\d{1,3}){0,2}))");
        if (output.isEmpty())
            return "";

        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch())
            return match.captured(1);

        return "";
    };

    QProcess process;
    process.start(python, { "--version" });
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString version = getVersion(output);
    if (version.isEmpty()) {
        output = process.readAllStandardError();
        version = getVersion(output);
        if (version.isEmpty()) {
            int index = python.lastIndexOf('/') + 1;
            output = python.mid(index, python.length() - index);
            version = getVersion(output);
        }
    }
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            + "/.unioncode/packages/Python" + version;
}

// setting from clangd trismit
QProcess *createCxxServ(const newlsp::ProjectKey &key)
{
    lspServOut << __FUNCTION__ << QCoreApplication::instance()->thread() << QThread::currentThread();
    if (key.language != newlsp::Cxx)
        return nullptr;

    QStringList procAs;
    QString clangd = "clangd-13";
    if (ProcessUtil::exists(clangd)) {
        procAs << clangd;
    } else {
        procAs << "clangd";
    }

    procAs << "--log=verbose";
    procAs << QString("--compile-commands-dir=%0").arg(key.outputDirectory.c_str());
    procAs << "--clang-tidy";
    procAs << "--completion-style=bundled";
    procAs << "--limit-results=500";

    auto proc = new QProcess();
    proc->setProgram("/usr/bin/bash");
    proc->setArguments({ "-c", procAs.join(" ") });

    return proc;
}

// setting from jdtls trismit
QProcess *createJavaServ(const newlsp::ProjectKey &key)
{
    lspServOut << __FUNCTION__ << qApp->thread() << QThread::currentThread();
    if (key.language != newlsp::Java)
        return nullptr;

    QString dataDir = CustomPaths::projectCachePath(QString::fromStdString(key.workspace));
    QString runtimePath = CustomPaths::lspRuntimePath(QString::fromStdString(key.language));
    bool noRuntimeChilds = QDir(runtimePath).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot).isEmpty();
    if (noRuntimeChilds) {
        if (!env::pkg::native::installed()) {
            RemoteChecker::instance().checkLanguageBackend(QString::fromStdString(key.language));
        } else {
            lspServOut << "unzip install native package..." << noRuntimeChilds;
            QString jdtlsNativePkgPath = env::pkg::native::path(env::package::Category::get()->jdtls);
            ProcessUtil::execute("tar", { "zxvf", jdtlsNativePkgPath, "-C", "." }, runtimePath,
                                 [=](const QByteArray &data) {
                                     lspServOut << QString(data);
                                 });
        }
    }

    auto proc = new QProcess();
    QString lanuchLine = "/usr/bin/java "
                         "-Declipse.application=org.eclipse.jdt.ls.core.id1 "
                         "-Dosgi.bundles.defaultStartLevel=4 "
                         "-Declipse.product=org.eclipse.jdt.ls.core.product "
                         "-Dlog.level=ALL "
                         "-noverify "
                         "-Xmx1G "
                         "--add-modules=ALL-SYSTEM "
                         "--add-opens java.base/java.util=ALL-UNNAMED "
                         "--add-opens java.base/java.lang=ALL-UNNAMED "
                         "-jar "
            + runtimePath + "/plugins/org.eclipse.equinox.launcher_1.6.400.v20210924-0641.jar "
                            "-configuration "
            + runtimePath + "/config_linux "
            + QString("-data %0").arg(dataDir);
    proc->setProgram("/usr/bin/bash");
    proc->setArguments({ "-c", lanuchLine });

    return proc;
}

// setting from pyls trismit
QProcess *createPythonServ(const newlsp::ProjectKey &key)
{
    lspServOut << __FUNCTION__ << qApp->thread() << QThread::currentThread();
    if (key.language != newlsp::Python)
        return nullptr;

    auto proc = new QProcess();
    auto env = proc->processEnvironment();
    env.insert("PYTHONPATH", packageInstallPath("python3"));
    proc->setProcessEnvironment(env);
    proc->setProgram("python3");
    proc->setArguments({ "-m", "pylsp", "-v" });

    return proc;
}

// setting from pyls trismit
QProcess *createJSServ(const newlsp::ProjectKey &key)
{
    lspServOut << "create js language server," << qApp->thread() << QThread::currentThread();
    if (key.language != newlsp::JS)
        return nullptr;

    QString jsServerInstallPath = CustomPaths::lspRuntimePath(QString::fromStdString(key.language));
    RemoteChecker::instance().checkJSServer(jsServerInstallPath);

    QString nodePath = jsServerInstallPath + "/node_modules/node/bin/node";
    QString serverPath = jsServerInstallPath + "/node_modules/.bin/typescript-language-server";
    if (!QFileInfo::exists(nodePath) || !QFileInfo::exists(serverPath)) {
        return nullptr;
    }

    auto proc = new QProcess();
    proc->setProgram(nodePath);
    proc->setArguments({ serverPath, "--stdio" });

    return proc;
}

void selectLspServer(const QJsonObject &params)
{
    QString language = params.value(QString::fromStdString(newlsp::language)).toString();
    QString workspace = params.value(QString::fromStdString(newlsp::workspace)).toString();
    QString output = params.value(QString::fromStdString(newlsp::output)).toString();
    newlsp::ProjectKey projectKey { language.toStdString(), workspace.toStdString(), output.toStdString() };
    JsonRpcCallProxy::ins().setSelect(projectKey);
    QProcess *proc = JsonRpcCallProxy::ins().value(projectKey);

    if (!proc) {
        proc = JsonRpcCallProxy::ins().createLspServ(projectKey);
        if (proc) {
            proc->setProcessChannelMode(QProcess::ForwardedOutputChannel);
            proc->start();
            lspServOut << "save backend process";
            JsonRpcCallProxy::ins().save(projectKey, proc);
            lspServOut << "selected ProjectKey{language:" << projectKey.language
                       << ", workspace:" << projectKey.workspace << "}";
            JsonRpcCallProxy::ins().setSelect(projectKey);
        }
    }

    if (!proc)
        lspServOut << "selected error ProjectKey{language:" << projectKey.language
                   << ",workspace:" << projectKey.workspace << "}";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    JsonRpcCallProxy::ins().bindCreateProc(newlsp::Cxx, createCxxServ);
    JsonRpcCallProxy::ins().bindCreateProc(newlsp::Java, createJavaServ);
    JsonRpcCallProxy::ins().bindCreateProc(newlsp::Python, createPythonServ);
    JsonRpcCallProxy::ins().bindCreateProc(newlsp::JS, createJSServ);
    JsonRpcCallProxy::ins().bindFilter("selectLspServer", selectLspServer);

    newlsp::ServerApplication lspServ(a);
    QObject::connect(&lspServ, &newlsp::ServerApplication::jsonrpcMethod,
                     &JsonRpcCallProxy::ins(), &JsonRpcCallProxy::methodFilter,
                     Qt::QueuedConnection);

    QObject::connect(&lspServ, &newlsp::ServerApplication::jsonrpcNotification,
                     &JsonRpcCallProxy::ins(), &JsonRpcCallProxy::notificationFilter,
                     Qt::QueuedConnection);
    lspServ.start();
    lspServOut << "created ServerApplication";

    return a.exec();
}
