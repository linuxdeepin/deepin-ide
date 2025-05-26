// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORSTYLE_H
#define EDITORSTYLE_H

#include "common/common_global.h"

#include <QString>

namespace support_file {

struct COMMON_EXPORT EditorStyle
{
    static QString globalPath(const QString &languageID);
    static QString userPath(const QString &languageID);
};

}

#endif // EDITORSTYLE_H
