// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICES_GLOBAL_H
#define SERVICES_GLOBAL_H

#if defined(SERVICES_LIBRARY)
#  define SERVICE_EXPORT Q_DECL_EXPORT
#else
#  define SERVICE_EXPORT Q_DECL_IMPORT
#endif


#endif // SERVICES_GLOBAL_H
