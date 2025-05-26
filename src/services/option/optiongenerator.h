// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONGENERATOR_H
#define OPTIONGENERATOR_H

#include "common/common.h"
#include "services/services_global.h"

#include <QWidget>

namespace dpfservice {

class SERVICE_EXPORT OptionGenerator : public Generator
{
    Q_OBJECT
public:
    OptionGenerator(){}
    virtual ~OptionGenerator(){}
    virtual QWidget *optionWidget(){ return nullptr; }
};

} // namespace dpfservice

#endif // OPTIONGENERATOR_H
