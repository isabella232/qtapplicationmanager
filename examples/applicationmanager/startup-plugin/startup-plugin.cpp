// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "startup-plugin.h"

Q_LOGGING_CATEGORY(LogMe, "am.start")

void TestStartupInterface::initialize(const QVariantMap &systemProperties) Q_DECL_NOEXCEPT_EXPR(false)
{
    qCWarning(LogMe) << "Startup initialize - systemProperties:" << systemProperties;
}

void TestStartupInterface::afterRuntimeRegistration() Q_DECL_NOEXCEPT_EXPR(false)
{
    qCWarning(LogMe) << "Startup afterRuntimeRegistration";
}

void TestStartupInterface::beforeQmlEngineLoad(QQmlEngine *engine) Q_DECL_NOEXCEPT_EXPR(false)
{
    qCWarning(LogMe) << "Startup beforeQmlEngineLoad - engine:" << engine;
}

void TestStartupInterface::afterQmlEngineLoad(QQmlEngine *engine) Q_DECL_NOEXCEPT_EXPR(false)
{
    qCWarning(LogMe) << "Startup afterQmlEngineLoad - engine:" << engine;
}

void TestStartupInterface::beforeWindowShow(QWindow *window) Q_DECL_NOEXCEPT_EXPR(false)
{
    qCWarning(LogMe) << "Startup beforeWindowShow - window:" << window;
}

void TestStartupInterface::afterWindowShow(QWindow *window) Q_DECL_NOEXCEPT_EXPR(false)
{
    qCWarning(LogMe) << "Startup afterWindowShow - window:" << window;
}
