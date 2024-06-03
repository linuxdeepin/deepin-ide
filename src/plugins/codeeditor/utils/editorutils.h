// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORUTILS_H
#define EDITORUTILS_H

#include <QAction>

class EditorUtils : public QObject
{
    Q_OBJECT
public:
    static int nbDigitsFromNbLines(long nbLines);
    static void registerShortcut(QAction *act, const QString &id, const QKeySequence &shortCut);
};

#endif   // EDITORUTILS_H
