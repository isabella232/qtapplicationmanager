// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QSortFilterProxyModel>
#include <QJSValue>
#include <QtAppManCommon/global.h>

QT_FORWARD_DECLARE_CLASS(QJSEngine);

QT_BEGIN_NAMESPACE_AM

class Intent;
class IntentModelPrivate;

class IntentModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_CLASSINFO("AM-QmlType", "QtApplicationManager.SystemUI/IntentModel 2.0")
    Q_CLASSINFO("AM-QmlPrototype", "QObject")

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QJSValue filterFunction READ filterFunction WRITE setFilterFunction NOTIFY filterFunctionChanged)
    Q_PROPERTY(QJSValue sortFunction READ sortFunction WRITE setSortFunction NOTIFY sortFunctionChanged)

public:
    IntentModel(QObject *parent = nullptr);

    int count() const;

    QJSValue filterFunction() const;
    void setFilterFunction(const QJSValue &callback);

    QJSValue sortFunction() const;
    void setSortFunction(const QJSValue &callback);

    Q_INVOKABLE int indexOfIntent(const QString &intentId, const QString &applicationId,
                                  const QVariantMap &parameters = {}) const;
    Q_INVOKABLE int indexOfIntent(QT_PREPEND_NAMESPACE_AM(Intent) *intent);
    Q_INVOKABLE int mapToSource(int ourIndex) const;
    Q_INVOKABLE int mapFromSource(int sourceIndex) const;

protected:
    using QSortFilterProxyModel::mapToSource;
    using QSortFilterProxyModel::mapFromSource;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

signals:
    void countChanged();
    void filterFunctionChanged();
    void sortFunctionChanged();

private:
    QJSEngine *getJSEngine() const;

    IntentModelPrivate *d;
};

QT_END_NAMESPACE_AM
