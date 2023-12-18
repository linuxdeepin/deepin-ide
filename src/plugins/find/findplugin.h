// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDPLUGIN_H
#define FINDPLUGIN_H

#include <framework/framework.h>

namespace dpfservice {
    class WindowService;
}

class FindPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "findplugin.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

signals:
    void asyncStopFinished();
    void onFindActionTriggered();

private:
    void sendSwitchSearchResult();
    dpfservice::WindowService *windowService = nullptr;
};

#endif // FINDPLUGIN_H
