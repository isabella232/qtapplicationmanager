// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QStringList>
#include <QVariantMap>
#include <QtAppManCommon/global.h>

QT_BEGIN_NAMESPACE_AM

class Intent : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("AM-QmlType", "QtApplicationManager.SystemUI/IntentObject 2.0 UNCREATABLE")

    Q_PROPERTY(QString intentId READ intentId CONSTANT)
    Q_PROPERTY(QString packageId READ packageId CONSTANT)
    Q_PROPERTY(QString applicationId READ applicationId CONSTANT)
    Q_PROPERTY(QT_PREPEND_NAMESPACE_AM(Intent)::Visibility visibility READ visibility CONSTANT)
    Q_PROPERTY(QStringList requiredCapabilities READ requiredCapabilities CONSTANT)
    Q_PROPERTY(QVariantMap parameterMatch READ parameterMatch CONSTANT)

    Q_PROPERTY(QUrl icon READ icon CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariantMap names READ names CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QVariantMap descriptions READ descriptions CONSTANT)
    Q_PROPERTY(QStringList categories READ categories CONSTANT)

public:
    enum Visibility {
        Public,
        Private
    };
    Q_ENUM(Visibility)

    Intent();

    QString intentId() const;
    Visibility visibility() const;
    QStringList requiredCapabilities() const;
    QVariantMap parameterMatch() const;

    QString packageId() const;
    QString applicationId() const;

    bool checkParameterMatch(const QVariantMap &parameters) const;

    QUrl icon() const;
    QString name() const;
    QVariantMap names() const;
    QString description() const;
    QVariantMap descriptions() const;
    QStringList categories() const;

private:
    Intent(const QString &intentId, const QString &packageId, const QString &applicationId,
           const QStringList &capabilities, Intent::Visibility visibility,
           const QVariantMap &parameterMatch, const QMap<QString, QString> &names,
           const QMap<QString, QString> &descriptions, const QUrl &icon,
           const QStringList &categories);

    QString m_intentId;
    Visibility m_visibility = Private;
    QStringList m_requiredCapabilities;
    QVariantMap m_parameterMatch;

    QString m_packageId;
    QString m_applicationId;

    QMap<QString, QString> m_names; // language -> name
    QMap<QString, QString> m_descriptions; // language -> description
    QStringList m_categories;
    QUrl m_icon;

    friend class IntentServer;
    friend class IntentServerHandler;
    friend class TestPackageLoader; // for auto tests only
};

QT_END_NAMESPACE_AM

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE_AM(Intent *))
