// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylescijava.h"
#include "textedittabwidget/style/stylejsonfile.h"
#include "textedittabwidget/style/stylecolor.h"
#include "textedittabwidget/textedit.h"
#include "SciLexer.h"

StyleSciJava::StyleSciJava(TextEdit *parent)
    : StyleSci (parent)
{

}

QMap<int, QString> StyleSciJava::keyWords() const
{
    return StyleSci::keyWords();
}

void StyleSciJava::setStyle()
{
    StyleSci::setStyle();
}

void StyleSciJava::setLexer()
{
    StyleSci::setLexer();
}

int StyleSciJava::sectionEnd() const
{
    return SCE_JAVA_ESCAPESEQUENCE;
}

void StyleSciJava::setThemeColor(DGuiApplicationHelper::ColorType colorType)
{
    StyleSci::setThemeColor(colorType);

    if (colorType == DGuiApplicationHelper::DarkType) {
        QJsonObject tempObj;
        int tempFore;
        auto jsonFile = edit()->getStyleFile();
        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Comment).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_JAVA_COMMENT, tempFore); // #整行
        edit()->styleSetFore(SCE_JAVA_COMMENTLINE, tempFore); // //注释
        edit()->styleSetFore(SCE_JAVA_COMMENTDOC, tempFore);
        edit()->styleSetFore(SCE_JAVA_COMMENTLINEDOC, tempFore); // ///注释
        edit()->styleSetFore(SCE_JAVA_COMMENTDOCKEYWORD, tempFore - 0x3333);
        edit()->styleSetFore(SCE_JAVA_COMMENTDOCKEYWORDERROR, tempFore - 0x3333); // /// @
        edit()->styleSetFore(SCE_JAVA_PREPROCESSORCOMMENT, tempFore);
        edit()->styleSetFore(SCE_JAVA_PREPROCESSORCOMMENTDOC, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Number).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_JAVA_NUMBER, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Keyword).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_JAVA_WORD, tempFore);
        edit()->styleSetFore(SCE_JAVA_WORD2, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->String).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_JAVA_STRING, tempFore); // 字符串
        edit()->styleSetFore(SCE_JAVA_CHARACTER, tempFore); // 字符串
        edit()->styleSetFore(SCE_JAVA_UUID, tempFore);
        edit()->styleSetFore(SCE_JAVA_STRINGEOL, tempFore);
        edit()->styleSetFore(SCE_JAVA_REGEX, tempFore);
        edit()->styleSetFore(SCE_JAVA_STRINGRAW, tempFore);
        edit()->styleSetFore(SCE_JAVA_VERBATIM, tempFore);
        edit()->styleSetFore(SCE_JAVA_HASHQUOTEDSTRING, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Preprocessor).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_JAVA_PREPROCESSOR, tempFore); // #

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Operators).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_JAVA_OPERATOR, tempFore); // 符号

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_JAVA_IDENTIFIER, tempFore);
        edit()->styleSetFore(SCE_JAVA_USERLITERAL, tempFore);
        edit()->styleSetFore(SCE_JAVA_TASKMARKER, tempFore);
        edit()->styleSetFore(SCE_JAVA_ESCAPESEQUENCE, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Global).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_JAVA_GLOBALCLASS, tempFore);
    } else {
        edit()->styleSetFore(SCE_JAVA_COMMENT, StyleColor::color(StyleColor::Table::get()->DeepSkyBlue)); // #整行
        edit()->styleSetFore(SCE_JAVA_COMMENTLINE, StyleColor::color(StyleColor::Table::get()->DarkTurquoise)); // //注释
        edit()->styleSetFore(SCE_JAVA_COMMENTDOC, StyleColor::color(StyleColor::Table::get()->DarkTurquoise));
        edit()->styleSetFore(SCE_JAVA_NUMBER, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_JAVA_WORD, StyleColor::color(StyleColor::Table::get()->Gold));
        edit()->styleSetFore(SCE_JAVA_STRING, StyleColor::color(StyleColor::Table::get()->Magenta)); // 字符串
        edit()->styleSetFore(SCE_JAVA_CHARACTER, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_JAVA_UUID, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_JAVA_PREPROCESSOR, StyleColor::color(StyleColor::Table::get()->MediumBlue)); // #
        edit()->styleSetFore(SCE_JAVA_OPERATOR, edit()->styleFore(SCE_JAVA_DEFAULT)); // 符号
        edit()->styleSetFore(SCE_JAVA_IDENTIFIER, edit()->styleFore(SCE_JAVA_DEFAULT));
        edit()->styleSetFore(SCE_JAVA_STRINGEOL, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_JAVA_VERBATIM, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_JAVA_REGEX, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_JAVA_COMMENTLINEDOC, StyleColor::color(StyleColor::Table::get()->DarkTurquoise)); // ///注释
        edit()->styleSetFore(SCE_JAVA_WORD2, StyleColor::color(StyleColor::Table::get()->Gold)); // 1 一般关键字
        edit()->styleSetFore(SCE_JAVA_COMMENTDOCKEYWORD, StyleColor::color(StyleColor::Table::get()->DeepSkyBlue));
        edit()->styleSetFore(SCE_JAVA_COMMENTDOCKEYWORDERROR, StyleColor::color(StyleColor::Table::get()->DeepSkyBlue)); // /// @
        edit()->styleSetFore(SCE_JAVA_GLOBALCLASS, StyleColor::color(StyleColor::Table::get()->Gold));
        edit()->styleSetFore(SCE_JAVA_STRINGRAW, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_JAVA_HASHQUOTEDSTRING, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_JAVA_PREPROCESSORCOMMENT, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_JAVA_PREPROCESSORCOMMENTDOC, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_JAVA_USERLITERAL, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_JAVA_TASKMARKER, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_JAVA_ESCAPESEQUENCE, StyleColor::color(StyleColor::Table::get()->Gainsboro));
    }
}
