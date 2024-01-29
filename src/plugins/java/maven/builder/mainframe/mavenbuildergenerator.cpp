// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavenbuildergenerator.h"
#include "maven/builder/parser/mavenparser.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

class MavenBuilderGeneratorPrivate
{
    friend class MavenBuilderGenerator;
};

MavenBuilderGenerator::MavenBuilderGenerator()
    : d(new MavenBuilderGeneratorPrivate())
{

}

MavenBuilderGenerator::~MavenBuilderGenerator()
{
    if (d)
        delete d;
}

BuildCommandInfo MavenBuilderGenerator::getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo)
{
    BuildCommandInfo info;
    info.kitName = projectInfo.kitName();
    info.workingDir = projectInfo.workspaceFolder();
    info.program = projectInfo.buildProgram();
    if (info.program.isEmpty())
        info.program = OptionManager::getInstance()->getMavenToolPath();
    switch (buildMenuType) {
    case Build:
        info.arguments.append("compile");
        break;
    case Clean:
        info.arguments.append("clean");
        break;
    }

    return info;
}

void MavenBuilderGenerator::appendOutputParser(std::unique_ptr<AbstractOutputParser> &outputParser)
{
    if (outputParser) {
        outputParser->takeOutputParserChain();
        outputParser->appendOutputParser(new MavenParser());
    }
}

bool MavenBuilderGenerator::checkCommandValidity(const BuildCommandInfo &info, QString &retMsg)
{
    if (info.program.trimmed().isEmpty()) {
        retMsg = tr("The build command of %1 project is null! "\
                    "please install it in console with \"sudo apt install maven\", and then restart the tool.")
                .arg(info.kitName.toUpper());
        return false;
    }

    if (!QFileInfo(info.workingDir.trimmed()).exists()) {
        retMsg = tr("The path of \"%1\" is not exist! "\
                    "please check and reopen the project.")
                .arg(info.workingDir);
        return false;
    }

    return true;
}
