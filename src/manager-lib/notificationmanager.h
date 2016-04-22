/****************************************************************************
**
** Copyright (C) 2016 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Pelagicore Application Manager.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
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
** SPDX-License-Identifier: GPL-3.0
**
****************************************************************************/

#pragma once

#include <QObject>
#include <QAbstractListModel>

QT_FORWARD_DECLARE_CLASS(QQmlEngine)
QT_FORWARD_DECLARE_CLASS(QJSEngine)


class NotificationManagerPrivate;

class NotificationManager : public QAbstractListModel
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    ~NotificationManager();
    static NotificationManager *createInstance();
    static NotificationManager *instance();
    static QObject *instanceForQml(QQmlEngine *qmlEngine, QJSEngine *);

    // the item model part
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE int count() const;
    Q_INVOKABLE QVariantMap get(int index) const;

    Q_INVOKABLE void acknowledgeNotification(int id);
    Q_INVOKABLE void triggerNotificationAction(int id, const QString &actionId);
    Q_INVOKABLE void dismissNotification(int id);

    // vv libnotify DBus interface
    Q_SCRIPTABLE QString GetServerInformation(QString &vendor, QString &version, QString &spec_version);
    Q_SCRIPTABLE QStringList GetCapabilities();
    Q_SCRIPTABLE uint Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int timeout);
    Q_SCRIPTABLE void CloseNotification(uint id);

signals:
    Q_SCRIPTABLE void NotificationClosed(uint id, uint reason);
    Q_SCRIPTABLE void ActionInvoked(uint id, const QString &action_key);
    // ^^ libnotify DBus interface

signals:
    void countChanged();

private:
    NotificationManager(QObject *parent = 0);
    NotificationManager(const NotificationManager &);
    NotificationManager &operator=(const NotificationManager &);
    static NotificationManager *s_instance;

    NotificationManagerPrivate *d;
    friend class NotificationManagerPrivate;
};
