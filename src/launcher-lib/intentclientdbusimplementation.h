// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QPointer>
#include <QtAppManIntentClient/intentclientsysteminterface.h>

class IoQtApplicationManagerIntentInterfaceInterface;

QT_BEGIN_NAMESPACE_AM

class IntentClientDBusImplementation : public IntentClientSystemInterface
{
public:
    IntentClientDBusImplementation(const QString &dbusName);

    void initialize(IntentClient *intentClient) Q_DECL_NOEXCEPT_EXPR(false) override;

    QString currentApplicationId(QObject *hint) override;

    void requestToSystem(QPointer<IntentClientRequest> icr) override;
    void replyFromApplication(QPointer<IntentClientRequest> icr) override;

private:
    IoQtApplicationManagerIntentInterfaceInterface *m_dbusInterface;
    QString m_dbusName;
};

QT_END_NAMESPACE_AM
