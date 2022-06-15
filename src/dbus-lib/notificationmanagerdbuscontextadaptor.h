// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtAppManDBus/abstractdbuscontextadaptor.h>

QT_BEGIN_NAMESPACE_AM

class NotificationManager;

class NotificationManagerDBusContextAdaptor : public AbstractDBusContextAdaptor
{
public:
    explicit NotificationManagerDBusContextAdaptor(NotificationManager *nm);
};

QT_END_NAMESPACE_AM
