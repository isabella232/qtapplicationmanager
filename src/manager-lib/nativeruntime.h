// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <qglobal.h>

#include <QtPlugin>
#include <QVector>

#include <QtAppManManager/abstractruntime.h>
#include <QtAppManManager/abstractcontainer.h>
#include <QtAppManManager/amnamespace.h>

QT_FORWARD_DECLARE_CLASS(QDBusConnection)
QT_FORWARD_DECLARE_CLASS(QDBusServer)

QT_BEGIN_NAMESPACE_AM

class Notification;
class NativeRuntime;
class NativeRuntimeInterface;
class NativeRuntimeApplicationInterface;
class NativeRuntimeManager : public AbstractRuntimeManager
{
    Q_OBJECT
public:
    explicit NativeRuntimeManager(QObject *parent = nullptr);
    explicit NativeRuntimeManager(const QString &id, QObject *parent = nullptr);

    static QString defaultIdentifier();
    bool supportsQuickLaunch() const override;

    AbstractRuntime *create(AbstractContainer *container, Application *app) override;
};

class NativeRuntime : public AbstractRuntime
{
    Q_OBJECT

public:
    ~NativeRuntime() override;

    bool isQuickLauncher() const override;
    bool attachApplicationToQuickLauncher(Application *app) override;

    qint64 applicationProcessId() const override;
    void openDocument(const QString &document, const QString &mimeType) override;
    void setSlowAnimations(bool slow) override;

    bool sendNotificationUpdate(Notification *n);

public slots:
    bool start() override;
    void stop(bool forceKill = false) override;

signals:
    void aboutToStop(); // used for the ApplicationInterface
    void interfaceCreated(const QString &interfaceName);

    void applicationConnectedToPeerDBus(const QDBusConnection &connection,
                                        QT_PREPEND_NAMESPACE_AM(Application) *application);
    void applicationReadyOnPeerDBus(const QDBusConnection &connection,
                                    QT_PREPEND_NAMESPACE_AM(Application) *application);
    void applicationDisconnectedFromPeerDBus(const QDBusConnection &connection,
                                             QT_PREPEND_NAMESPACE_AM(Application) *application);

private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, Am::ExitStatus status);
    void onProcessError(Am::ProcessError error);
    void onDBusPeerConnection(const QDBusConnection &connection);
    void onApplicationFinishedInitialization();

protected:
    explicit NativeRuntime(AbstractContainer *container, Application *app, NativeRuntimeManager *parent);

private:
    bool initialize();
    void shutdown(int exitCode, Am::ExitStatus status);
    QDBusServer *applicationInterfaceServer() const;
    bool startApplicationViaLauncher();

    bool m_isQuickLauncher;
    bool m_startedViaLauncher;

    QString m_document;
    QString m_mimeType;
    bool m_connectedToApplicationInterface = false;
    bool m_dbusConnection = false;
    QString m_dbusConnectionName;

    NativeRuntimeApplicationInterface *m_applicationInterface = nullptr;
    NativeRuntimeInterface *m_runtimeInterface = nullptr;
    AbstractContainerProcess *m_process = nullptr;
    QDBusServer *m_applicationInterfaceServer;
    bool m_slowAnimations = false;
    QVariantMap m_openGLConfiguration;

    friend class NativeRuntimeManager;
};

QT_END_NAMESPACE_AM
