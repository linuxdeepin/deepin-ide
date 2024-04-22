// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "notificationentity.h"

typedef std::shared_ptr<NotificationEntity> EntityPtr;

static const int ItemSpacing = 10;   //消息通知内部Space
static const int StatusBarPadding = 5;   //最下方通知距状态栏间隔
static const int BubbleTimeout = 5000;   //通知默认超时时间(毫秒)
static const int BubbleEntities = 5;
static const char DefaultButtonField[] = "_default";
static const char NotificationContentName[] = "NotificationContent";

#endif   // CONSTANTS_H
