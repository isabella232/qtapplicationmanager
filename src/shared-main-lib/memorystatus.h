/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2019 Luxoft Sweden AB
** Copyright (C) 2018 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtApplicationManager module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once

#include <QtAppManCommon/global.h>

#include <QtAppManMonitor/systemreader.h>

#include <QObject>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE_AM

class MemoryStatus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("AM-QmlType", "QtApplicationManager/MemoryStatus 2.0")
    Q_PROPERTY(quint64 totalMemory READ totalMemory CONSTANT)
    Q_PROPERTY(quint64 memoryUsed READ memoryUsed NOTIFY memoryUsedChanged)

    Q_PROPERTY(QStringList roleNames READ roleNames CONSTANT)

public:
    MemoryStatus(QObject *parent = nullptr);

    quint64 totalMemory() const;
    quint64 memoryUsed() const;

    QStringList roleNames() const;

    Q_INVOKABLE void update();

signals:
    void memoryUsedChanged();

private:
    QScopedPointer<MemoryReader> m_memoryReader;
    quint64 m_memoryUsed;
};

QT_END_NAMESPACE_AM

