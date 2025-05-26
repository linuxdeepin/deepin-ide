// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONCHATGENERATOR_H
#define OPTIONCHATGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionCustomModelsGenerator : public dpfservice::OptionGenerator
{
    Q_OBJECT
public:
    OptionCustomModelsGenerator();
    inline static QString kitName() { return tr("Models"); }
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONCHATGENERATOR_H
