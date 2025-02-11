// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

#include <QtAppManCommon/global.h>

QT_FORWARD_DECLARE_CLASS(QDataStream)

QT_BEGIN_NAMESPACE_AM

class YamlPackageScanner;
class PackageInfo;

class IntentInfo
{
public:
    IntentInfo(PackageInfo *packageInfo);
    ~IntentInfo();

    static const quint32 DataStreamVersion;

    enum Visibility {
        Public,
        Private
    };

    QString id() const;
    Visibility visibility() const;
    QStringList requiredCapabilities() const;
    QVariantMap parameterMatch() const;
    QString handlingApplicationId() const;

    QStringList categories() const;

    QMap<QString, QString> names() const;
    QMap<QString, QString> descriptions() const;
    QString icon() const;

    void writeToDataStream(QDataStream &ds) const;
    static IntentInfo *readFromDataStream(PackageInfo *pkg, QDataStream &ds);

private:
    PackageInfo *m_packageInfo;
    QString m_id;
    Visibility m_visibility = Public;
    QStringList m_requiredCapabilities;
    QVariantMap m_parameterMatch;

    QString m_handlingApplicationId;
    QStringList m_categories;
    QMap<QString, QString> m_names; // language -> name
    QMap<QString, QString> m_descriptions; // language -> description
    QString m_icon; // relative to the manifest's location

    friend class YamlPackageScanner;
    Q_DISABLE_COPY(IntentInfo)
};

QT_END_NAMESPACE_AM
