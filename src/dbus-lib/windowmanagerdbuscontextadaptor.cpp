// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "windowmanagerdbuscontextadaptor.h"
#include "windowmanager.h"
#include "windowmanager_adaptor.h"

QT_BEGIN_NAMESPACE_AM

WindowManagerDBusContextAdaptor::WindowManagerDBusContextAdaptor(WindowManager *am)
    : AbstractDBusContextAdaptor(am)
{
    m_adaptor = new WindowManagerAdaptor(this);
}

QT_END_NAMESPACE_AM

/////////////////////////////////////////////////////////////////////////////////////

QT_USE_NAMESPACE_AM

WindowManagerAdaptor::WindowManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{ }

WindowManagerAdaptor::~WindowManagerAdaptor()
{ }

bool WindowManagerAdaptor::runningOnDesktop() const
{
    return WindowManager::instance()->isRunningOnDesktop();
}

bool WindowManagerAdaptor::slowAnimations() const
{
    return WindowManager::instance()->slowAnimations();
}

void WindowManagerAdaptor::setSlowAnimations(bool slow)
{
    WindowManager::instance()->setSlowAnimations(slow);
}

bool WindowManagerAdaptor::makeScreenshot(const QString &filename, const QString &selector)
{
    return WindowManager::instance()->makeScreenshot(filename, selector);
}
