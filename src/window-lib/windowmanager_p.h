// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QVector>
#include <QMap>
#include <QHash>

#include <QtAppManWindow/windowmanager.h>

QT_FORWARD_DECLARE_CLASS(QQmlEngine)

QT_BEGIN_NAMESPACE_AM

class WindowManagerPrivate
{
public:
    int findWindowBySurfaceItem(QQuickItem *quickItem) const;

#if defined(AM_MULTI_PROCESS)
    int findWindowByWaylandSurface(QWaylandSurface *waylandSurface) const;

    WaylandCompositor *waylandCompositor = nullptr;
    QVector<int> extraWaylandSockets;

    static QString applicationId(Application *app, WindowSurface *windowSurface);
#endif

    QHash<int, QByteArray> roleNames;

    // All windows, regardless of content state, that haven't been released (hence destroyed) yet.
    QVector<Window *> allWindows;

    // Windows that are actually exposed by the model to the QML side.
    // Only windows whose content state is different than Window::NoSurface are
    // kept here.
    QVector<Window *> windowsInModel;

    bool shuttingDown = false;
    bool slowAnimations = false;
    bool allowUnknownUiClients = false;

    QList<QQuickWindow *> views;
    QString waylandSocketName;
    QQmlEngine *qmlEngine;
};

QT_END_NAMESPACE_AM
// We mean it. Dummy comment since syncqt needs this also for completely private Qt modules.
