// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavenparser.h"

#include "common/type/task.h"
#include "common/util/fileutils.h"

const char TASK_CATEGORY_BUILDSYSTEM[] = "Task.Category.Buildsystem";

MavenParser::MavenParser()
{
    setObjectName(QLatin1String("MavenParser"));
}

void MavenParser::stdOutput(const QString &line, OutputPane::OutputFormat format)
{
    QString newContent = line;
    QRegExp exp("\\033\\[(\\d*;*\\d*)m");
    newContent.replace(exp, "");

    if (newContent.indexOf("[ERROR]") != -1) {
        format = OutputPane::OutputFormat::ErrorMessage;
        stdError(newContent);
    }

    emit outputAdded(newContent, format);

    AbstractOutputParser::stdOutput(newContent, format);
}

void MavenParser::stdError(const QString &line)
{
    QString newContent = line;
    QRegExp exp("/.*:\\[(\\d*),(\\d*)\\]");
    int pos = newContent.indexOf(exp);
    QString filePath;
    int lineNumber = -1;
    if (pos != -1) {
        QStringList list = newContent.mid(pos).split(":");
        if (list.count() > 1) {
            filePath = list.at(0);
            QString last = list.at(1);
            QStringList sublist = last.split(",");
            if (sublist.count() > 1) {
                lineNumber = sublist[0].mid(1).toInt();
            }
        }
    } else {
        QRegExp pomExp("Non-parseable POM /.*:");
        QString header = "Non-parseable POM ";
        pos = newContent.indexOf(pomExp);
        if (pos != -1) {
            newContent = newContent.mid(pos);
            pos = newContent.indexOf(":");
            if (pos != -1) {
                filePath = newContent.left(pos).mid(header.length() - 1);
                newContent = newContent.mid(pos);
                pos = newContent.indexOf("@ line ");
                if (pos != -1) {
                    newContent = newContent.mid(pos);
                    pos = newContent.indexOf(",");
                    if (pos != -1) {
                        newContent = newContent.left(pos);
                        header = "@ line ";
                        lineNumber = newContent.mid(header.length() - 1).toInt();
                    }
                }
            }
        }
    }

    Utils::FileName fileName;
    if (QFileInfo(filePath).isFile()) {
        fileName = Utils::FileName::fromUserInput(filePath);
    } else {
        fileName = Utils::FileName();
    }

    taskAdded(Task(Task::Error,
                   line,
                   fileName,
                   lineNumber,
                   TASK_CATEGORY_BUILDSYSTEM), 1, 0);
}

void MavenParser::taskAdded(const Task &task, int linkedLines, int skippedLines)
{
    emit addTask(task, linkedLines, skippedLines);
}
